# ESPHome Custom Components


## elsner_weather_station

Should work with the following devices:
- elsner elektronik P03/3-RS485 basic
- elsner elektronik P03/3-RS485-CET
- elsner elektronik P04/3-RS485 basic
- elsner elektronik P04/3-RS485-CET
- Eltako Multisensor MS

Tested on ESP32 (Olimex ESP32 POE) with Eltako Multisensor MS.


## zehnder_comfoair_q

Work in progress and heavily based on:
- https://github.com/vekexasia/comfoair-esp32
- https://github.com/marco-hoyer/zcan
- https://github.com/michaelarnauts/comfoconnect (good documentation there also if you want to add additional sensors etc.)

Needs at least ESPHome 2022.5.0 (since it depends on some CAN bus component updates).

Tested only on ESP32 (Olimex ESP32 POE) using it's internal CAN bus component, but might also work on an ESP8266 with an MCP2551 (untested).
