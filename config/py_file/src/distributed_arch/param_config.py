import unitlib
import base_arch.param_config as param_config
import copy

class ParamConfig(param_config.ParamConfig): 
    def gen_params(self, hierarchy, instances, arch_config):
        self._gen_base_params(hierarchy, arch_config)
        self._gen_perfect_params(hierarchy, arch_config)
        self._modify_config_params(arch_config)
        self._gen_fu_params(hierarchy, instances, arch_config)
        self._gen_rank_params(hierarchy, instances)
        self._pmu(hierarchy = hierarchy,
                  On = True)
        
    def _pmu(self, hierarchy, On):
        self._modify_unit_params(hierarchy, unitlib.units.pmu, unitlib.params.pmu.turn_on, On)
        return hierarchy
        
    def _gen_base_params(self, hierarchy, arch_config):
        # global param
        issue_width = 8
        self._modify_param(hierarchy, "issue_width", issue_width)
        self._modify_param(hierarchy, "queue_depth", 2 * issue_width)
        self._modify_param(hierarchy, "phy_reg_num", 32 * issue_width)
        # unit param
        self._modify_unit_params(hierarchy, unitlib.units.rob, unitlib.params.rob.queue_depth, 32 * issue_width)
        self._modify_unit_params(hierarchy, unitlib.units.rob, unitlib.params.rob.retire_heartbeat, 100000)
        
        self._modify_unit_params(hierarchy, unitlib.units.dispatch_stage, unitlib.params.dispatch_stage.queue_depth, 32)
        
        self._modify_unit_params(hierarchy, unitlib.units.scheduler, unitlib.params.scheduler.queue_depth, 32 * issue_width)
        self._modify_unit_params(hierarchy, unitlib.units.scheduler, unitlib.params.scheduler.issue_width, int(issue_width/len(arch_config.dispatch_map)))
        
        self._modify_unit_params(hierarchy, unitlib.units.physical_regfile, unitlib.params.physical_regfile.latency, 0)
        
        # self._modify_unit_params(hierarchy, unitlib.units.perfect_fu, unitlib.params.scheduler.issue_width, 2)
        self._modify_unit_params(hierarchy, unitlib.units.perfect_fu, unitlib.params.scheduler.queue_depth, 40)
        self._modify_unit_params(hierarchy, unitlib.units.perfect_fu, unitlib.params.scheduler.issue_width, int(issue_width/len(arch_config.dispatch_map)))
        return hierarchy
    
    def _gen_perfect_params(self, hierarchy, arch_config):
        # global param
        mem_ptr_num = 3000000
        self._modify_unit_params(hierarchy, unitlib.units.self_allocators, unitlib.params.self_allocators.inst_max_num, mem_ptr_num)
        self._modify_unit_params(hierarchy, unitlib.units.self_allocators, unitlib.params.self_allocators.inst_arch_info_max_num, mem_ptr_num)
        self._modify_unit_params(hierarchy, unitlib.units.self_allocators, unitlib.params.self_allocators.inst_group_pair_max_num, mem_ptr_num)
        self._modify_unit_params(hierarchy, unitlib.units.self_allocators, unitlib.params.self_allocators.inst_group_max_num, mem_ptr_num)
        self._modify_unit_params(hierarchy, unitlib.units.self_allocators, unitlib.params.self_allocators.credit_pair_max_num, mem_ptr_num)
        
        self._modify_param(hierarchy, "phy_reg_num", int(mem_ptr_num / 2))
        # unit param
        
        self._modify_unit_params(hierarchy, unitlib.units.rob, unitlib.params.rob.queue_depth, int(mem_ptr_num / 2))
        
        self._modify_unit_params(hierarchy, unitlib.units.dispatch_stage, unitlib.params.dispatch_stage.queue_depth, 128)
        
        self._modify_unit_params(hierarchy, unitlib.units.scheduler, unitlib.params.scheduler.queue_depth, int(mem_ptr_num / 4))
        
        # infinite issue
        # issue_width = 1000
        
        # self._modify_unit_params(hierarchy, unitlib.units.rob, unitlib.params.rob.retire_heartbeat, 1000)
        
        # self._modify_param(hierarchy, "issue_width", issue_width)
        # self._modify_unit_params(hierarchy, unitlib.units.dispatch_stage, unitlib.params.dispatch_stage.queue_depth, issue_width * 5)
        # self._modify_unit_params(hierarchy, unitlib.units.renaming_stage, unitlib.params.renaming_stage.queue_depth, issue_width * 2)
        # self._modify_unit_params(hierarchy, unitlib.units.physical_regfile, unitlib.params.physical_regfile.queue_depth, issue_width * 2)
        # self._modify_unit_params(hierarchy, unitlib.units.perfect_fu, unitlib.params.perfect_fu.queue_depth, issue_width * 2)
        
    def _modify_config_params(self, arch_config):
        arch_config.fu_latency[unitlib.func_type.LDU] = 4
    

    def _gen_rank_params(self, hierarchy, instances):
        for unit_name, count_map in instances["instance_topo"]["table"].items():
            for count, instance_name in count_map.items():
                self._modify_instance_param(hierarchy, instance_name, "pipe_rank", count)
        
        
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
                        self._modify_unit_params(tmp_hierarchy_3, unitlib.units.scheduler, unitlib.params.scheduler.queue_depth, scheduler_size)
                        file_name_3 = f"{file_name_2}_rs{scheduler_size}"
                        
                        hierarchy_map[file_name_3] = tmp_hierarchy_3

        return hierarchy_map