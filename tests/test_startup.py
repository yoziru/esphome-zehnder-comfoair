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
    bus = yaml.safe_load((ROOT / "packages/canbus.yml").read_text())["canbus"][0]
    bus.update(platform="test_canbus", id="test_bus")
    del bus["tx_pin"], bus["rx_pin"]
    actions = [
        {"lambda": (ROOT / "tests/startup_checks.cpp").read_text()},
        {"delay": "1500ms"},
        {"lambda": '''
            esphome::test_canbus::check(id(test_bus).sent.empty(),
                "feedback must not leave delayed outbound commands");
        '''},
    ]
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
                    esphome::test_canbus::check(sent.size() == 2 && sent[1].data[6] == {expected},
                        "thermostat after synchronization: {contract}");
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
        "substitutions": {"filter_time": "1s", "filter_send_every": "1", "filter_window_size": "1"},
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
        "zehnder_comfoair_q": {"id": "comfoair", "canbus_id": "test_bus", "request_ids": [65, 67]},
    }
    BUILD.mkdir(parents=True, exist_ok=True)
    path = BUILD / f"{name}.yaml"
    includes = "packages:\n" + "".join(
        f"  {p}: !include {ROOT / 'packages' / (p + '.yml')}\n" for p in packages
    )
    path.write_text(yaml.safe_dump(config, sort_keys=False) + includes)
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
