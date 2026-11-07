# Preheater Telemetry

The shared CLI and Device Builder packages request and expose:

| PDO | Reading | Representation |
| --- | --- | --- |
| 144 | Pre-heater Energy YTD | UINT16, kWh |
| 145 | Pre-heater Energy Since Start | UINT16, kWh |
| 146 | Pre-heater Power | UINT16, W |

Each PDO must contain exactly two little-endian bytes. Wrong-width frames do not
replace the last valid reading. No response remains unknown; a device-supplied
zero is a valid reading but does not establish whether a preheater is installed.
No heater controls, forced heating, or installation detection are added.

`Pre-heater Power` preserves the latest value reported by Zehnder, without
averaging. `Pre-heater Power (Smoothed)` is a standard ESPHome copy sensor using
the configured averaging interval and moving-window settings used by ventilation
power. It adds no CAN requests and does not change the source reading.

Preserving the original reading matters for diagnostics and observing heater
transitions or short peaks. Averaging delays changes and may produce a value
that was never reported in an individual frame. It is less representative of
the latest reading, not necessarily less accurate as an estimate of average load.
The moving window retains recent samples; these are not synchronized measurements
or a guaranteed fixed-duration average when PDO updates are sparse.

The existing `Power` entity remains smoothed ventilation power with its same
name/ID. `Ventilation + Pre-heater Power (Smoothed)` adds it to the smoothed
preheater reading, not to the unsmoothed source. Missing/non-finite sources make
the derived readings unavailable instead of becoming zero; pending averages
cannot replace an invalidated source with a valid-looking value. This is not
total device power: electronics, display, and other loads are not included.

The energy counters retain the existing `total_increasing` convention. Year
resets and 16-bit wraps are not converted into a synthetic lifetime accumulator.
Physical active-heating, calibration, and counter-reset behavior need separate
hardware verification; host tests do not prove these properties.

Based on the preheater sensor work in
[PR #63 by ArturDevCraft](https://github.com/yoziru/esphome-zehnder-comfoair/pull/63).
Units/types match the pinned
[aiocomfoconnect reference](https://github.com/michaelarnauts/aiocomfoconnect/blob/8fe395b3f8657e25ef8c5a6ccea5aab06808a514/aiocomfoconnect/sensors.py).
