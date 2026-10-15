import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, light
from esphome.const import CONF_ADDRESS, CONF_OUTPUT_ID

DEPENDENCIES = ["i2c"]

led_ns = cg.esphome_ns.namespace("led_i2c")
LedI2C = led_ns.class_("LedI2C", cg.Component, light.LightOutput, i2c.I2CDevice)

CONFIG_SCHEMA = (
    light.RGB_LIGHT_SCHEMA.extend(
        {
            cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(LedI2C),
            cv.Optional("command_interval", default="10ms"): cv.All(
                cv.positive_time_period_milliseconds,
                cv.Range(min=cv.TimePeriod(milliseconds=1), max=cv.TimePeriod(seconds=60)),
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(i2c.i2c_device_schema(0x46))
    .extend({
        cv.Optional(CONF_ADDRESS, default=0x46): cv.All(
            cv.i2c_address, cv.int_range(min=0x08, max=0x77)
        ),
    })
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)
    await light.register_light(var, config)
    cg.add(var.set_command_interval(config["command_interval"].total_milliseconds))
