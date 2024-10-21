import unitlib
import base_arch.unit_config as unit_config

class SimpleUnitConfig(unit_config.UnitConfig):
    def _gen_hierarchy(self, is_gen_static_units):
        hierarchy = {}
        hierarchy["info"] = []
        
        hierarchy["core0"] = {}
        
        hierarchy["core0"]["frontend"] = []
        hierarchy["core0"]["backend"] = []
        hierarchy["core0"]["func_units"] = []
        hierarchy["core0"]["global_ctrl"] = []
        hierarchy["core0"]["cache"] = []
        hierarchy["core0"]["memory"] = []
        
        hierarchy["info"].append(unitlib.units.self_allocators)
        hierarchy["info"].append(unitlib.units.mavis)
        hierarchy["info"].append(unitlib.units.global_param)
        hierarchy["info"].append(unitlib.units.pmu)
    
        hierarchy["core0"]["frontend"].append(unitlib.units.perfect_frontend)
        
        hierarchy["core0"]["backend"].append(unitlib.units.renaming_stage)
        hierarchy["core0"]["backend"].append(unitlib.units.dispatch_stage)
        hierarchy["core0"]["backend"].append(unitlib.units.rob)
        hierarchy["core0"]["backend"].append(unitlib.units.physical_regfile)
        hierarchy["core0"]["backend"].append(unitlib.units.reservation_station)
        
        hierarchy["core0"]["func_units"].append(unitlib.units.perfect_fu)
        hierarchy["core0"]["func_units"].append(unitlib.units.write_back_stage)
        
        hierarchy["core0"]["global_ctrl"].append(unitlib.units.flush_manager)
            
        return hierarchy