import unitlib
import base_arch.instance_config as instance_config

class SimpleInstanceConfig(instance_config.InstanceConfig):
    def _gen_instance(self, units_map):
        instances = {}
        for unit_name, unit_info_map in units_map.items():
            match unit_name:
                case unitlib.units.reservation_station | unitlib.units.perfect_fu:
                    instances[unit_name] = {}
                    for instance_count in range(self.arch_config.dispatch_path_num):
                        instances[unit_name][f"{unit_name}_{instance_count}"] = {}
                        instances[unit_name][f"{unit_name}_{instance_count}"].update(units_map[unit_name]["params"])
                case _:
                    instances[unit_name] = {}
                    instances[unit_name][unit_name] = {}
                    instances[unit_name][unit_name].update(units_map[unit_name]["params"])
        
        return instances