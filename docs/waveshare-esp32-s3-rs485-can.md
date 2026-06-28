# Connecting Zehnder ComfoAir Q with Waveshare ESP32-S3-RS485-CAN

This board is a validated DIN-rail option for this project. It has been tested
successfully with the Zehnder ComfoAir Q configuration in this repository using
the unit's ComfoNet screw terminals for CAN and 12 V power.

## Why this board is relevant

- DIN-rail ABS enclosure, suitable for a short wall-mounted rail near the unit
- Built-in isolated CAN interface on screw terminals
- Built-in isolated RS485 interface on screw terminals
- 7-36 V DC power input on screw terminals

The RS485 interface is not used by the current Zehnder ComfoAir Q ESPHome
configuration, but it may be useful for additional equipment in the same
cabinet.

## ESPHome pin mapping

| Function | Pin |
| --- | --- |
| CAN TX | `GPIO15` |
| CAN RX | `GPIO16` |
| RS485 TX | `GPIO17` |
| RS485 RX | `GPIO18` |
| RS485 TX enable / DE | `GPIO21` |
| I2C SCL | `GPIO38` |
| I2C SDA | `GPIO39` |
| Header GPIO1 | `GPIO1` |
| Header GPIO2 | `GPIO2` |

Expected board substitutions:

```yaml
substitutions:
  can_tx_pin: GPIO15
  can_rx_pin: GPIO16
  rs485_tx_pin: GPIO17
  rs485_rx_pin: GPIO18
  rs485_flow_control_pin: GPIO21
  rs485_baud_rate: "9600"
  i2c_sda_pin: GPIO39
  i2c_scl_pin: GPIO38
  header_gpio1_pin: GPIO1
  header_gpio2_pin: GPIO2
  board: esp32-s3-devkitc-1
  variant: esp32s3
  flash_size: 16MB
```

Board-specific component IDs exposed by this variant:

| Feature | ESPHome ID |
| --- | --- |
| RS485 UART | `waveshare_rs485` |
| I2C bus | `waveshare_i2c` |
| PCF85063 RTC | `waveshare_rtc` |

Build this variant with:

```sh
make validate-config BOARD=waveshare-esp32-s3-rs485-can
make compile BOARD=waveshare-esp32-s3-rs485-can
```

## Zehnder CAN wiring

If a verified 12 V supply from the ventilation unit is available, it can power
the Waveshare board directly through the `DC+` / `DC-` power terminal.

```

|----------------+                                +------------------------------+
|                |                                |                              |
|   [ComfoAir]   |                                | [Waveshare ESP32-S3-RS485-CAN]
|                |                                |                              |
|             12V o-------------------------------o DC+                          |
|             GND o-------------------------------o DC-                          |
|           CAN_H o-------------------------------o CAN H                        |
|           CAN_L o-------------------------------o CAN L                        |
|----------------+                                +------------------------------+
```

The tested setup uses the ComfoNet screw terminals:

| ComfoNet terminal | Signal | Waveshare terminal |
| --- | --- | --- |
| red / 12V | +12 V | `DC+` |
| black / GND | 0 V / GND | `DC-` |
| yellow / CAN_H | CAN-H | `H` |
| white / CAN_L | CAN-L | `L` |

This terminal wiring has been validated with the Waveshare board and the normal
ESPHome project.

## RJ45 ComfoNet wiring

Zehnder documents ComfoNet as a combined CAN bus and 12 V supply. The official
ComfoConnect documentation gives this signal set and color code for the
4-core ComfoNet cable:

| Signal | Official ComfoNet color |
| --- | --- |
| CAN_L | white |
| CAN_H | yellow |
| GND | black |
| 12V | red |

For the ComfoAir Q RJ45 socket, existing community projects document the
following RJ45 pin mapping. The signal pins stay the same, but wire colors
depend on whether the cable is wired as T568A or T568B.

T568B cable colors:

| T568B RJ45 pin | RJ45 wire | Signal | Waveshare terminal |
| --- | --- | --- | --- |
| 1 | white-orange | CAN-L | `L` |
| 2 | orange | CAN-H | `H` |
| 3 | white-green | GND | `DC-` |
| 4 | blue | +12 V | `DC+` |

T568A cable colors:

| T568A RJ45 pin | RJ45 wire | Signal | Waveshare terminal |
| --- | --- | --- | --- |
| 1 | white-green | CAN-L | `L` |
| 2 | green | CAN-H | `H` |
| 3 | white-orange | GND | `DC-` |
| 4 | blue | +12 V | `DC+` |

Only use the RJ45 12 V line after verifying the actual cable and socket with a
multimeter. Do not connect unused RJ45 conductors. For stripped patch cables,
identify the RJ45 pins with a continuity tester instead of relying on wire color
alone; crossover, mixed-standard, or non-network cables may not follow the color
tables above.

## Powering from the unit's 12 V supply

Direct 12 V power is suitable because Waveshare specifies the power screw
terminal for 7-36 V DC input. Use this wiring only after checking the real
connector:

- `+12 V` to `DC+`
- `0 V` / `GND` to `DC-`
- CAN-H to `H`
- CAN-L to `L`

Before permanent operation:

- Verify the 12 V polarity with a multimeter.
- Verify that the 12 V output has enough spare current for the board.
- Add a small inline fuse if the source is not already suitably protected.

## Mounting notes

The DIN-rail enclosure is still useful without a full cabinet. Practical options
for this project are:

- on a short wall-mounted DIN rail near the Zehnder unit
- on a small DIN-rail plate fixed next to the service opening
- lying inside the Zehnder unit only if it is mechanically secured, kept away
  from moving parts, airflow paths, condensate, and serviceable high-voltage
  areas

For Wi-Fi reliability, avoid mounting the board behind large metal covers. If
the signal is weak, test the location with the enclosure closed before final
mounting.

## CAN notes

- Waveshare's demo default CAN bitrate is 250 kbit/s.
- This repository uses 50 kbit/s for Zehnder ComfoAir Q in `packages/canbus.yml`.
- Use the project bitrate, not the Waveshare demo default.
- The board has a reserved 120 ohm CAN termination option, but Waveshare
  documents it as not connected by default.
- The CAN bus must be correctly terminated for reliable operation.
- CAN tests without another device at the same bitrate commonly produce ACK or
  bus errors.

## Sources

- Waveshare Wiki: https://www.waveshare.com/wiki/ESP32-S3-RS485-CAN
- Product page: https://www.waveshare.com/esp32-s3-rs485-can.htm
- Demo ZIP: https://files.waveshare.com/wiki/ESP32-S3-RS485-CAN/ESP32-S3-RS485-CAN-Demo.zip
- Schematic: https://files.waveshare.com/wiki/ESP32-S3-RS485-CAN/ESP32-S3-RS485-CAN-Schematic.pdf
- Zehnder ComfoAir Q service manual: https://zehnderamerica.com/wp-content/uploads/2024/08/ComfoAir_Q_Service_Zehnder-America_EN_2022.05.18.pdf
- Zehnder ComfoConnect LAN C manual: https://www.thebuiltenvironment.co.uk/wp-content/uploads/2023/07/A.-Zehnder-LAN-C-User-and-Installation-Manual.pdf
- Zehnder ComfoConnect PRO technical sheet: https://dokumenty.maro.cz/prilohy/Zehnder_ComfoConnect_PRO_471429300_technicky_list.pdf
- dan-s-github Zehnder ComfoAir CAN RJ45 wiring: https://github.com/dan-s-github/zehnder-comfoair-can/blob/main/M5STACK_CAN_BUS_KIT.md
- dan-s-github custom PCB RJ45 and terminal wiring: https://github.com/dan-s-github/zehnder-comfoair-can/blob/main/CUSTOM_PCB.md
