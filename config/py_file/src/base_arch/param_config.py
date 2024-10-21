import unitlib

class ParamConfig:
    def __init__(self) -> None:
        pass
    
    def gen_params(self, hierarchy, instances, arch_config):
        return instances
    
    def gen_all_exploration_params(self, hierarchy, instances, arch_config):
        return
    
    def _gen_fu_params(self, hierarchy, instances, arch_config):
        dispatch_map = []
        for (instance_name, param), fu_type_array in \
            zip(instances[unitlib.units.reservation_station].items(), arch_config.get_dispatch_map()):
            dispatch_map.append(instance_name)
            dispatch_map.append("|")
            dispatch_map.append(f"{param[unitlib.params.reservation_station.issue_width]}")
            dispatch_map.append("|")
            dispatch_map = dispatch_map + fu_type_array
            dispatch_map.append("|")
            
            
        self._modify_unit_params(hierarchy, unitlib.units.global_param, unitlib.params.global_param.dispatch_map, dispatch_map)
            
        write_back_map = []
        for instance_name, param in instances[unitlib.units.perfect_fu].items():
            write_back_map.append(instance_name)
            write_back_map.append("|")
            write_back_map.append(f"{param[unitlib.params.perfect_fu.issue_width]}")
            write_back_map.append("|")
        
        self._modify_unit_params(hierarchy, unitlib.units.global_param, unitlib.params.global_param.write_back_map, write_back_map)
            
        fu_latency_map = arch_config.get_fu_latency()
        
        self._modify_unit_params(hierarchy, unitlib.units.global_param, unitlib.params.global_param.fu_latency_map, fu_latency_map)
            
        return hierarchy
    
    def _modify_param(self, map, param_name, data, find = False):
        if isinstance(map, dict):
            for key, value in map.items():
                if isinstance(value, dict):
                    self._modify_param(value, param_name, data)
                elif isinstance(value, list):
                    if key == param_name:
                        map[key] = data
                    else:
                        self._modify_param(value, param_name, data)
                elif key == param_name:
                    map[key] = data
        elif isinstance(map, list):
            for member in map:
                if isinstance(member, dict) or isinstance(member, list):
                    self._modify_param(member, param_name, data)
        return

    def _modify_unit_params(self, map, unit_name, param_name, data, find = False):
        if isinstance(map, dict):
            for key, value in map.items():
                if isinstance(value, dict):
                    if key == unit_name and not find:
                        find = True
                        self._modify_unit_params(value, unit_name, param_name, data, find)
                        find = False
                    else:
                        self._modify_unit_params(value, unit_name, param_name, data, find)
                elif isinstance(value, list):
                    if not find or param_name != key:
                        self._modify_unit_params(value, unit_name, param_name, data, find)
                    elif not value:
                        map[key] = data
                    elif isinstance(value[0], str):
                        map[key] = data
                    else:
                        self._modify_unit_params(value, unit_name, param_name, data, find)
                elif key == param_name and find:
                    map[key] = data
        elif isinstance(map, list):
            for member in map:
                if isinstance(member, dict) or isinstance(member, list):
                    if member == unit_name and not find:
                        find = True
                        if self._modify_unit_params(member, unit_name, param_name, data, find):
                            map[key] = data
                        find = False
                    else:
                        self._modify_unit_params(member, unit_name, param_name, data, find)
        return
    
    def _modify_instance_param(self, map, instance_name, param_name, data, find = False):
        self._modify_unit_params(map, instance_name, param_name, data, find)
        return