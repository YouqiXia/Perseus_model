import simple_arch.simple_instance_config as instance_config
import simple_arch.simple_param_config as param_config
import simple_arch.simple_port_config as port_config
import simple_arch.simple_unit_config as unit_config
import simple_arch.simple_arch_config as arch_config

class SimpleConfigFactory:
    def __init__(self):
        pass
    
    def gen_unit_config(self, units):
        return unit_config.SimpleUnitConfig(units)

    def gen_arch_config(self):
        return arch_config.SimpleArchConfig()
    
    def gen_instance_config(self, config_units, config_arch):
        return instance_config.SimpleInstanceConfig(config_units, config_arch)
    
    def gen_param_config(self):
        return param_config.SimpleParamConfig()
    
    def gen_port_config(self, units, units_map, instances):
        return port_config.SimplePortConfig(units, units_map, instances)

