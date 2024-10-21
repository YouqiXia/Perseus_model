class InstanceConfig:
    def __init__(self, config_units_, config_arch_):
        self.arch_config = config_arch_
        self.instances = self._gen_instance(config_units_.get_units())
        self.hierarchy = {}
        self._gen_hierarchy(config_units_.get_hierarchy(), self.hierarchy)
        pass
    
    def get_instances(self):
        return self.instances
    
    def get_hierarchy(self):
        return self.hierarchy
    
    def _gen_instance(self, units_map):
        instances = {}
        return instances
    
    def _gen_hierarchy(self, map, hierarchy):
        for key, value in map.items():
            if isinstance(value, dict):
                hierarchy[key] = {}
                self._gen_hierarchy(value, hierarchy[key])
            elif isinstance(value, list):
                hierarchy[key] = []
                for unit_name in value:
                    hierarchy[key].append({unit_name: self.instances[unit_name]})
        return 
    