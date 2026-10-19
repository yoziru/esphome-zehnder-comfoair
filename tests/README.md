# Host C++ regression tests

PDO type/width, integer boundary, and unknown-type checks run in the same
`make test` host suite, including the sanitizer and iterator-bounds configurations.

Protocol framing, sequence rollover and RTR scheduling are also covered. See
`test-audit.md` for the contract inventory. Run `ESPHOME_TEST_SANITIZE=1 make test`
for ASan/UBSan; the existing CI test job runs normal and sanitized configurations.

Run `make test` with uv and a C++ compiler installed. ESPHome is pinned by
`requirements.txt`; PlatformIO's native target builds and runs the actual C++
component, ESPHome entities, and production YAML automations. No ESP32 or unit
is needed. Only the physical CAN transport is fake: it supplies PDO frames and
records transmitted command frames. Generated files stay in `tests/.esphome`.

The same checks run with and without climate, on a fresh boot and a second boot
with saved ESPHome preferences. They protect read-only startup/PDO feedback and
verify that explicit select, switch, and fan controls still transmit commands.
This deliberately tests the owning boundary rather than a reimplementation of
ESPHome restore, polling, or trigger behavior.

## Issue #59 findings (ESPHome 2026.9.1)

Before the fix, the host reproduction fails because:

- Template switches default to `ALWAYS_OFF`. During `setup()`, Auto Ventilation's
  OFF action sends `84 15 08 01 ... 01` (enable permanent manual mode), and Away's
  OFF action sends `85 15 01 0b` (cancel Away). This happens without climate.
- PDO 65 updates `fan_level_raw`, whose automation calls `fan.turn_on`. The fan's
  ON trigger then turns on Auto Ventilation. In Manual (limited), this immediately
  enables permanent manual mode, followed by a delayed command to return to Auto.
  Feedback must not be treated as a user request, regardless of PDO arrival order.
- With climate included, thermostat setup also selects Normal before a profile
  PDO arrives. Guarding only climate does not fix the other two paths.

The fix disables local switch/fan restore for device-owned state, prevents the
fan ON automation from echoing a matching PDO, and guards the thermostat's idle
action until a profile exists. Heat/cool already require a real temperature;
guarding those on a separate PDO could swallow a legitimate thermostat transition
if the temperature arrives first. It does not globally block user commands or CAN requests.

The reported **temperature reset with climate removed is not reproduced** by the
stock packages. `TemplateTextSensor::update()` returns immediately without a
lambda, both before and after receiving Warm; `update_interval: never` cannot
directly prevent a profile write through that implementation. These tests verify
that behavior but do not establish why the workaround helped OP. Full merged
configuration and CAN logs around boot are needed to resolve that discrepancy.
Likewise, the host proves unwanted mode commands, not the physical unit's
subsequent choice of Medium. Confirm final unit state on hardware before claiming
all symptoms are resolved.
