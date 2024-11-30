import simple_arch.instance_config as instance_config
import simple_arch.param_config as param_config
import simple_arch.port_config as port_config
import simple_arch.unit_config as unit_config
import simple_arch.arch_config as arch_config

class SimpleConfigFactory:
    def __init__(self):
        pass
    
    def gen_unit_config(self, units):
        return unit_config.UnitConfig(units)

    def gen_arch_config(self):
        return arch_config.ArchConfig()
    
    def gen_instance_config(self, config_units, config_arch):
        return instance_config.InstanceConfig(config_units, config_arch)
    
    def gen_param_config(self):
        return param_config.ParamConfig()
    
    def gen_port_config(self, units, units_map, instances):
        return port_config.PortConfig(units, units_map, instances)

