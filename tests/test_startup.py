"""Compile and run actual ESPHome C++/YAML on the host, without an ESP32."""

import os
from pathlib import Path
import subprocess
import sys
import tempfile

import yaml

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / "tests/.esphome/startup"


def run(climate):
    name = "startup-climate" if climate else "startup-no-climate"
    packages = ["binary_sensors", "sensors", "text_sensors", "select", "switch", "fan"]
    if climate:
        packages.append("climate")
    packages.append("modulated_fan_speed")
    packages.append("airflow_constraints")
    packages.append("filter_lifetime")
    packages.append("temperature_profile_mode")
    bus = yaml.safe_load((ROOT / "packages/canbus.yml").read_text())["canbus"][0]
    bus.update(platform="test_canbus", id="test_bus")
    del bus["tx_pin"], bus["rx_pin"]
    actions = [
        {"lambda": (ROOT / "tests/startup_checks.cpp").read_text()},
        {"delay": "1500ms"},
        {"lambda": '''
            esphome::test_canbus::check(id(test_bus).sent.empty(),
                "feedback must not leave delayed outbound commands");
            auto &bus = *id(test_bus);
            bus.receive(146, std::vector<uint8_t>{0x20, 0x03});
            id(ventilation_and_preheater_power).update();
            esphome::test_canbus::check(std::isfinite(id(power).state) &&
                id(ventilation_and_preheater_power).state == id(power).state + 800,
                "combined sensor uses actual ventilation and decoded heater readings");
            id(pre_heater_power).publish_state(NAN);
            id(ventilation_and_preheater_power).update();
            esphome::test_canbus::check(std::isnan(id(ventilation_and_preheater_power).state),
                "ventilation-only feedback cannot invent missing preheater power");
            id(pre_heater_power).publish_state(INFINITY);
            id(ventilation_and_preheater_power).update();
            esphome::test_canbus::check(std::isnan(id(ventilation_and_preheater_power).state),
                "non-finite source invalidates combined power");
            bus.receive(146, std::vector<uint8_t>{0, 0});
            id(ventilation_and_preheater_power).update();
            esphome::test_canbus::check(id(ventilation_and_preheater_power).state == id(power).state && bus.sent.empty(),
                "zero heater restores the sum without settings writes");
        '''},
    ]
    for mode, offset in [(1, 2), (6, 1)]:
        actions.extend([
            {"lambda": f'''
                id(test_bus).receive(49, {mode});
                id(test_bus).sent.clear();
                id(auto_ventilation).turn_on();
            '''},
            {"delay": "2500ms"},
            {"lambda": f'''
                const auto &sent = id(test_bus).sent;
                esphome::test_canbus::check(sent.size() >= {offset + 2} &&
                    sent[{offset}].can_data_length_code == 4 && sent[{offset}].data[0] == 0x85 &&
                    sent[{offset}].data[2] == 0x08 && sent[{offset}].data[3] == 0x01 &&
                    sent[{offset + 1}].can_data_length_code == 4 && sent[{offset + 1}].data[0] == 0x85 &&
                    sent[{offset + 1}].data[2] == 0x01 && sent[{offset + 1}].data[3] == 0x01,
                    "Auto after delayed mode {mode} cancels both timers in order");
                id(test_bus).receive(49, 255);
                id(test_bus).sent.clear();
            '''},
        ])
    if climate:
        # Let the real thermostat's minimum run/off timers expire between actions.
        cases = [
            (0, "0xc8, 0x00", "HEAT", 2, "heating selects Warm"),  # 20.0 C
            (2, "0xe1, 0x00", None, 0, "idle selects Normal"),  # 22.5 C
            (0, "0x04, 0x01", "COOL", 1, "cooling selects Cool"),  # 26.0 C
        ]
        for profile, temperature, mode, expected, contract in cases:
            mode_call = "" if mode is None else f'''
                auto call = id(comfoair_climate).make_call();
                call.set_mode(climate::CLIMATE_MODE_{mode});
                call.perform();
            '''
            actions.extend([
                {"lambda": f'''
                    id(test_bus).receive(67, {profile});
                    id(test_bus).sent.clear();
                    id(test_bus).receive(274, std::vector<uint8_t>{{{temperature}}});
                    {mode_call}
                '''},
                {"delay": "1500ms"},
                {"lambda": f'''
                    const auto &sent = id(test_bus).sent;
                    esphome::test_canbus::check(sent.size() == 2 &&
                        sent[0].data[1] == 0x84 && sent[0].data[2] == 0x15 &&
                        sent[0].data[3] == 0x03 && sent[0].data[4] == 0x01 && sent[1].data[6] == {expected},
                        "thermostat after synchronization: {contract}");
                '''},
            ])
    actions.extend([
        {"lambda": '''
            auto &bus = *id(test_bus);
            bus.requests.clear();
            id(comfoair).set_request_ids({65, 67});
            id(comfoair).update();
            id(comfoair).update();
            esphome::test_canbus::check(bus.requests.size() == 1,
                "an update must not interleave a second request cycle");
        '''},
        {"delay": "300ms"},
        {"lambda": '''
            auto &requests = id(test_bus).requests;
            esphome::test_canbus::check(requests.size() == 2 &&
                requests[0].can_id == ((65u << 14) | 0x6a) &&
                requests[1].can_id == ((67u << 14) | 0x6a) &&
                requests[0].use_extended_id && requests[1].use_extended_id &&
                requests[0].remote_transmission_request && requests[1].remote_transmission_request,
                "delayed PDO cycle preserves order and sends extended RTR frames");
            id(comfoair).set_request_ids({});
            id(comfoair).update();
            esphome::test_canbus::check(requests.size() == 2, "empty request list sends nothing");
            id(comfoair).set_request_ids({65, 67});
            id(comfoair).update();
        '''},
        {"delay": "300ms"},
        {"lambda": '''
            const auto &requests = id(test_bus).requests;
            esphome::test_canbus::check(requests.size() == 4 &&
                requests[2].can_id == requests[0].can_id && requests[3].can_id == requests[1].can_id,
                "completed request cycle can start again");
        '''},
    ])
    actions.append({
        "lambda": '''
            esphome::test_canbus::check(global_preferences->sync(),
                "host preferences can be saved for a restart");
            std::exit(esphome::test_canbus::failures ? EXIT_FAILURE : EXIT_SUCCESS);
        '''
    })
    config = {
        "substitutions": {"filter_time": "1s", "filter_send_every": "1", "filter_window_size": "1", "comfoair_canbus_id": "test_bus"},
        "esphome": {
            "name": name,
            "build_path": str(BUILD / name),
            "on_boot": [{"priority": -100, "then": actions}],
        },
        "host": {},
        "logger": {"level": "DEBUG"},
        "external_components": [
            {"source": {"type": "local", "path": str(ROOT / "components")}},
            {"source": {"type": "local", "path": str(ROOT / "tests/components")}},
        ],
        "canbus": [bus],
        "zehnder_comfoair_q": {"id": "comfoair", "canbus_id": "test_bus", "request_ids": [65, 67], "request_interval": "never"},
    }
    if os.environ.get("ESPHOME_TEST_SANITIZE") == "1":
        config["esphome"]["platformio_options"] = {
            "build_flags": ["-fsanitize=address,undefined", "-fno-omit-frame-pointer"]
        }
    if os.environ.get("ESPHOME_TEST_BOUNDS") == "1":
        flags = (["-D_LIBCPP_ABI_BOUNDED_ITERATORS_IN_VECTOR=1",
                  "-D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_DEBUG"]
                 if sys.platform == "darwin" else ["-D_GLIBCXX_DEBUG"])
        config["esphome"].setdefault("platformio_options", {}).setdefault("build_flags", []).extend(flags)
    BUILD.mkdir(parents=True, exist_ok=True)
    path = BUILD / f"{name}.yaml"
    includes = "packages:\n" + "".join(
        f"  {p}: !include {ROOT / 'packages' / (p + '.yml')}\n" for p in packages
    )
    # Verify actual ESPHome merge semantics, not a copied expected source list.
    from esphome.config import read_config
    from esphome.core import CORE
    CORE.reset()
    CORE.config_path = path
    path.write_text(yaml.safe_dump(config, sort_keys=False) + includes)
    merged = read_config({})
    assert merged is not None
    shared = yaml.safe_load((ROOT / "packages/zehnder_comfoair_q.yml").read_text())
    assert {144, 145, 146} <= set(shared["zehnder_comfoair_q"]["request_ids"])
    assert sorted(int(pdo) for pdo in merged["zehnder_comfoair_q"]["request_ids"]) == [65, 67, 226, 230]
    assert len(merged["canbus"]) == 1
    subprocess.run([sys.executable, "-m", "esphome", "compile", str(path)], check=True)
    # Fresh preferences per run; do not inherit the workstation's host state.
    with tempfile.TemporaryDirectory(dir=BUILD) as prefs:
        for boot in ("fresh boot", "restart with saved preferences"):
            print(f"\n{name}: {boot}", flush=True)
            subprocess.run(
                [str(BUILD / name / ".pioenvs" / name / "program")],
                env={**os.environ, "ESPHOME_PREFDIR": prefs},
                check=True,
                timeout=15,
            )


if __name__ == "__main__":
    run(False)
    if "--no-climate" not in sys.argv:
        run(True)
