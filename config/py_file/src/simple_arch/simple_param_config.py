import unitlib
import base_arch.param_config as param_config
import copy

class SimpleParamConfig(param_config.ParamConfig): 
    def gen_params(self, hierarchy, instances, arch_config):
        self._gen_base_params(hierarchy, arch_config)
        self._gen_fu_params(hierarchy, instances, arch_config)
        self._pmu(hierarchy = hierarchy,
                  On = True)
        
    def _pmu(self, hierarchy, On):
        self._modify_unit_params(hierarchy, unitlib.units.pmu, unitlib.params.pmu.turn_on, On)
        return hierarchy
        
    def _gen_base_params(self, hierarchy, arch_config):
        # global param
        self._modify_param(hierarchy, "issue_width", 4)
        self._modify_param(hierarchy, "phy_reg_num", 128)
        # unit param
        self._modify_unit_params(hierarchy, unitlib.units.rob, unitlib.params.rob.queue_depth, 128)
        self._modify_unit_params(hierarchy, unitlib.units.rob, unitlib.params.rob.retire_heartbeat, 100000)
        
        self._modify_unit_params(hierarchy, unitlib.units.dispatch_stage, unitlib.params.dispatch_stage.queue_depth, 16)
        
        self._modify_unit_params(hierarchy, unitlib.units.reservation_station, unitlib.params.reservation_station.queue_depth, 128)
        
        self._modify_unit_params(hierarchy, unitlib.units.perfect_fu, unitlib.params.reservation_station.queue_depth, 20)
        return hierarchy

    def gen_all_exploration_params(self, hierarchy, instances, arch_config):
        # TODO: 改成递归调用
        hierarchy_map = {}
        # fu_latency_range = range(1, 5)
        fu_latency_range = [1, 4]
        phy_reg_num_range = [128 + i * 16 for i in range(2)]
        rob_size_range = [128 + i * 16 for i in range(2)]
        scheduler_size_range = [128 + i * 16 for i in range(2)]
        for fu_latency in fu_latency_range:
            file_name = "Genshin"
            tmp_arch_config = copy.deepcopy(arch_config)
            tmp_hierarchy = copy.deepcopy(hierarchy)
            tmp_instances = copy.deepcopy(instances)
        
            tmp_arch_config.modify_fu_latency(unitlib.func_type.LDU, fu_latency)
        
            self.gen_params(tmp_hierarchy, tmp_instances, tmp_arch_config)
            file_name = f"{file_name}_{unitlib.func_type.LDU}{fu_latency}"
        
            for phy_reg_num in phy_reg_num_range:
                tmp_hierarchy_1 = copy.deepcopy(tmp_hierarchy)
                self._modify_param(tmp_hierarchy_1, "phy_reg_num", phy_reg_num)
                file_name_1 = f"{file_name}_phy{phy_reg_num}"

                for rob_size in rob_size_range:
                    tmp_hierarchy_2 = copy.deepcopy(tmp_hierarchy_1)
                    self._modify_unit_params(tmp_hierarchy_2, unitlib.units.rob, unitlib.params.rob.queue_depth, rob_size)        
                    file_name_2 = f"{file_name_1}_rob{rob_size}"
            
                    for scheduler_size in scheduler_size_range:
                        tmp_hierarchy_3 = copy.deepcopy(tmp_hierarchy_2)
                        self._modify_unit_params(tmp_hierarchy_3, unitlib.units.reservation_station, unitlib.params.reservation_station.queue_depth, scheduler_size)
                        file_name_3 = f"{file_name_2}_rs{scheduler_size}"
                        
                        hierarchy_map[file_name_3] = tmp_hierarchy_3

        return hierarchy_map