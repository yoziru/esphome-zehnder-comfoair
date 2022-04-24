import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, sensor, binary_sensor
from esphome.const import (
    CONF_ILLUMINANCE,
    CONF_ID,
    CONF_TEMPERATURE,
    CONF_WIND_SPEED,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_ILLUMINANCE,
    UNIT_CELSIUS,
    UNIT_LUX,
    STATE_CLASS_MEASUREMENT,
)


DEPENDENCIES = ["uart"]
AUTO_LOAD = ["sensor", "binary_sensor"]


elsner_weather_station_ns = cg.esphome_ns.namespace("elsner_weather_station")
ElsnerWeatherStation = elsner_weather_station_ns.class_("ElsnerWeatherStation", cg.Component, uart.UARTDevice)

CONF_ALARM = "alarm"
CONF_IS_DARK = "is_dark"
CONF_IS_RAINING = "is_raining"
CONF_SUN_SOUTH = "sun_south"
CONF_SUN_WEST = "sun_west"
CONF_SUN_EAST = "sun_east"
CONF_WATCHDOG_TIMEOUT = "watchdog_timeout"
UNIT_KILOLUX = "klx"
UNIT_METER_PER_SECOND = "m/s"

SENSOR_TYPES = [
    CONF_TEMPERATURE,
    CONF_SUN_SOUTH,
    CONF_SUN_WEST,
    CONF_SUN_EAST,
    CONF_ILLUMINANCE,
    CONF_WIND_SPEED,
]
BINARY_SENSOR_TYPES = [
    CONF_ALARM,
    CONF_IS_DARK,
    CONF_IS_RAINING,
]


SUN_SCHEMA = sensor.sensor_schema(
    unit_of_measurement=UNIT_KILOLUX,
    accuracy_decimals=0,
    device_class=DEVICE_CLASS_ILLUMINANCE,
    state_class=STATE_CLASS_MEASUREMENT,
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(ElsnerWeatherStation),
            cv.Optional(CONF_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_SUN_SOUTH): SUN_SCHEMA,
            cv.Optional(CONF_SUN_WEST): SUN_SCHEMA,
            cv.Optional(CONF_SUN_EAST): SUN_SCHEMA,
            cv.Optional(CONF_IS_DARK): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_ILLUMINANCE): sensor.sensor_schema(
                unit_of_measurement=UNIT_LUX,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_ILLUMINANCE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_WIND_SPEED): sensor.sensor_schema(
                unit_of_measurement=UNIT_METER_PER_SECOND,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_IS_RAINING): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_ALARM): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_WATCHDOG_TIMEOUT, default="5s"): cv.positive_time_period_milliseconds,
        }
    )
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    for key in SENSOR_TYPES:
        if key in config:
            sens = await sensor.new_sensor(config[key])
            cg.add(getattr(var, f"set_{key}_sensor")(sens))

    for key in BINARY_SENSOR_TYPES:
        if key in config:
            sens = await binary_sensor.new_binary_sensor(config[key])
            cg.add(getattr(var, f"set_{key}_sensor")(sens))

    cg.add(var.set_watchdog_timeout(config[CONF_WATCHDOG_TIMEOUT]))
