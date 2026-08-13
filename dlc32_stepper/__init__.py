import esphome.codegen as cg
import esphome.config_validation as cv

dlc32_stepper_ns = cg.esphome_ns.namespace('dlc32_stepper')
DLC32Stepper = dlc32_stepper_ns.class_('DLC32Stepper', cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(DLC32Stepper),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[cv.CONF_ID])
    await cg.register_component(var, config)
