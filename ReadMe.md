# ESPHome Custom Components

## zehnder_comfoair_q
Still work in progress heavily based on:
- https://github.com/vekexasia/comfoair-esp32
- https://github.com/marco-hoyer/zcan
- https://github.com/michaelarnauts/comfoconnect (good documentation there also if you want to add additional sensors etc.)

Also needs [./components/canbus](./components/canbus) until https://github.com/esphome/esphome/pull/3376 has been merged.

Tested only on ESP32 (Olimex ESP32 POE) using it's internal CAN bus component, but might also work on an ESP8266 with an MCP2551 (untested).
