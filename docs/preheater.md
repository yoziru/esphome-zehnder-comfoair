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

The existing `Power` entity remains ventilation power with the same name/ID.
`Ventilation + Pre-heater Power` adds the two readings only when both are known;
otherwise it is unavailable. This is not total device power: electronics,
display, and other loads are not included. The sum uses the published ventilation
reading (including its existing smoothing) and the latest preheater reading.

The energy counters retain the existing `total_increasing` convention. Year
resets and 16-bit wraps are not converted into a synthetic lifetime accumulator.
Physical active-heating, calibration, and counter-reset behavior need separate
hardware verification; host tests do not prove these properties.

Based on the preheater sensor work in
[PR #63 by ArturDevCraft](https://github.com/yoziru/esphome-zehnder-comfoair/pull/63).
Units/types match the pinned
[aiocomfoconnect reference](https://github.com/michaelarnauts/aiocomfoconnect/blob/8fe395b3f8657e25ef8c5a6ccea5aab06808a514/aiocomfoconnect/sensors.py).
