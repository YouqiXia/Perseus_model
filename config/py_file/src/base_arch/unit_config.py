class UnitConfig:
    def __init__(self, units_) -> None:
        self.units_map = self._gen_units_map(units_.GetUnitsInfo())
        self.hierarchy = self._gen_hierarchy(True)
        self.hierarchy_without_details = self._gen_hierarchy(False)
        self._gen_hierarchy_back_from_units(self.hierarchy, "")
        pass
    
    def get_units(self):
        return self.units_map
    
    def get_hierarchy(self):
        return self.hierarchy
        
    def _gen_units_map(self, units):
        units_map = {}
        for unit_name, unit_statistics in units.items() :
            units_map.update({unit_name: {}})
            units_map[unit_name].update({"ports": unit_statistics.ports_info})
            units_map[unit_name].update({"params": {}})
        for unit_name, unit_statistics in units.items() :
            for param_name, parambase in unit_statistics.params_info.items():
                if ("bool" in parambase.getTypeName()):
                    param_value = parambase.getBoolResource()
                elif ("int64" in parambase.getTypeName()):
                    param_value = parambase.getInt64Resource()
                elif ("int32" in parambase.getTypeName()):
                    param_value = parambase.getInt32Resource()
                elif ("std::vector" in parambase.getTypeName()):
                    param_value = parambase.getVectorStringResource()
                elif ("std::string" in parambase.getTypeName()):
                    param_value = parambase.getStringResource()
                units_map[unit_name]["params"].update({param_name: param_value})
                
        return units_map

    def _gen_hierarchy(self, is_gen_static_units):
        hierarchy = {}
        return hierarchy
    
    def _gen_hierarchy_back_from_units(self, map, path):
        for key, value in map.items():
            if isinstance(value, dict):
                if path == "":
                    path = f"{key}"
                else:
                    path = f"{path}.{key}"
                self._gen_hierarchy_back_from_units(value, path)
            elif isinstance(value, list):
                for unit_name in value:
                    self.units_map[unit_name].update({"hierarchy": f"{path}.{key}"})
        return
        