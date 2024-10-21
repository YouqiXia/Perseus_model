import simple_arch.simple_instance_config as simple_instance_config
import simple_arch.simple_param_config as simple_param_config
import simple_arch.simple_port_config as simple_port_config
import simple_arch.simple_unit_config as simple_unit_config
import base_arch.arch_config as arch_config #TODO: separate arch into Simple arch or other arch

class SimpleConfigFactory:
    def __init__(self):
        pass
    
    def gen_unit_config(self, units):
        return simple_unit_config.SimpleUnitConfig(units)

    def gen_arch_config(self):
        return arch_config.SimpleArchConfig()
    
    def gen_instance_config(self, config_units, config_arch):
        return simple_instance_config.SimpleInstanceConfig(config_units, config_arch)
    
    def gen_param_config(self):
        return simple_param_config.SimpleParamConfig()
    
    def gen_port_config(self, units, units_map, instances):
        return simple_port_config.SimplePortConfig(units, units_map, instances)

