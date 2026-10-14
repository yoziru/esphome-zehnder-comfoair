import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import canbus
from esphome.const import CONF_ID

TestCanbus = cg.esphome_ns.namespace("test_canbus").class_(
    "TestCanbus", canbus.CanbusComponent
)
CONFIG_SCHEMA = canbus.CANBUS_SCHEMA.extend(
    {cv.GenerateID(): cv.declare_id(TestCanbus)}
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await canbus.register_canbus(var, config)
