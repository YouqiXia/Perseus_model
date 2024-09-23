import unitlib
import json

class Config:
    def __init__(self, units_) -> None:
        # the number of pipeline after disptach stage
        self.dispatch_path_num = 1
        
        self.units_map = self.gen_units_map(units_.GetUnitsInfo())
        self.unbind_ports = self.gen_ports(units_.GetUnitsInfo())
        
        self.instances = {}
        self.bindings = []
        self.hierarchy = self.gen_hierarchy(True)
        self.gen_hierarchy_back_from_units(self.hierarchy, "")
        
        self.process_instance()
        self.gen_binding_topo()
        
        self.gen_params()
        
        
    def gen_units_map(self, units):
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
    
    def gen_ports(self, units):
        ports_list = []
        for unit_name, unit_statistics in units.items() :
            for out_direction, ports in unit_statistics.ports_info.items():
                ports_list += ports
            
        return ports_list
    
    def process_instance(self):
        for unit_name, unit_info_map in self.units_map.items():
            match unit_name:
                case unitlib.units.reservation_station | unitlib.units.perfect_fu:
                    self.instances[unit_name] = {}
                    for instance_count in range(self.dispatch_path_num):
                        self.instances[unit_name][f"{unit_name}_{instance_count}"] = {}
                        self.instances[unit_name][f"{unit_name}_{instance_count}"].update(self.units_map[unit_name]["params"])
                case _:
                    self.instances[unit_name] = {}
                    self.instances[unit_name][unit_name] = {}
                    self.instances[unit_name][unit_name].update(self.units_map[unit_name]["params"])
                    
        return

    def gen_hierarchy(self, is_gen_static_units):
        hierarchy = {}
        hierarchy = {}
        hierarchy["info"] = []
        
        hierarchy["core0"] = {}
        
        hierarchy["core0"]["frontend"] = []
        hierarchy["core0"]["backend"] = []
        hierarchy["core0"]["func_units"] = []
        hierarchy["core0"]["global_ctrl"] = []
        hierarchy["core0"]["cache"] = []
        hierarchy["core0"]["memory"] = []
        
        hierarchy["info"].append(self.gen_units_pair(unitlib.units.self_allocators, is_gen_static_units))
        hierarchy["info"].append(self.gen_units_pair(unitlib.units.mavis, is_gen_static_units))
        hierarchy["info"].append(self.gen_units_pair(unitlib.units.global_param, is_gen_static_units))
    
        hierarchy["core0"]["frontend"].append(self.gen_units_pair(unitlib.units.perfect_frontend, is_gen_static_units))
        
        hierarchy["core0"]["backend"].append(self.gen_units_pair(unitlib.units.renaming_stage, is_gen_static_units))
        hierarchy["core0"]["backend"].append(self.gen_units_pair(unitlib.units.dispatch_stage, is_gen_static_units))
        hierarchy["core0"]["backend"].append(self.gen_units_pair(unitlib.units.rob, is_gen_static_units))
        hierarchy["core0"]["backend"].append(self.gen_units_pair(unitlib.units.physical_regfile, is_gen_static_units))
        hierarchy["core0"]["backend"].append(self.gen_units_pair(unitlib.units.reservation_station, is_gen_static_units))
        
        hierarchy["core0"]["func_units"].append(self.gen_units_pair(unitlib.units.perfect_fu, is_gen_static_units))
        hierarchy["core0"]["func_units"].append(self.gen_units_pair(unitlib.units.write_back_stage, is_gen_static_units))
        
        hierarchy["core0"]["global_ctrl"].append(self.gen_units_pair(unitlib.units.flush_manager, is_gen_static_units))
            
        return hierarchy
    
    def gen_units_pair(self, units_name, is_gen_static_units):
        tmp_map = {}
        if is_gen_static_units:
            return units_name
        else:
            tmp_map[units_name] = self.instances[units_name]
            return tmp_map
    
    def gen_hierarchy_back_from_units(self, map, path):
        for key, value in map.items():
            if isinstance(value, dict):
                if path == "":
                    path = f"{key}"
                else:
                    path = f"{path}.{key}"
                self.gen_hierarchy_back_from_units(value, path)
            elif isinstance(value, list):
                for unit_name in value:
                    self.units_map[unit_name].update({"hierarchy": f"{path}.{key}"})
        return
    
    def gen_params(self):
        dispatch_map = []
        for instance_name, param in self.instances[unitlib.units.reservation_station].items():
            dispatch_map.append(instance_name)
            dispatch_map.append("|")
            dispatch_map.append(f"{param[unitlib.params.reservation_station.issue_width]}")
            dispatch_map.append("|")
            dispatch_map.append(unitlib.func_type.ALU)
            dispatch_map.append(unitlib.func_type.BRU)
            dispatch_map.append(unitlib.func_type.CSR)
            dispatch_map.append(unitlib.func_type.MUL)
            dispatch_map.append(unitlib.func_type.DIV)
            dispatch_map.append(unitlib.func_type.FPU)
            dispatch_map.append(unitlib.func_type.LDU)
            dispatch_map.append(unitlib.func_type.STU)
            dispatch_map.append("|")
            
        self.instances[unitlib.units.global_param][unitlib.units.global_param]\
            [unitlib.params.global_param.dispatch_map] = dispatch_map
            
        write_back_map = []
        for instance_name, param in self.instances[unitlib.units.perfect_fu].items():
            write_back_map.append(instance_name)
            write_back_map.append("|")
            write_back_map.append(f"{param[unitlib.params.perfect_fu.issue_width]}")
            write_back_map.append("|")
        
        self.instances[unitlib.units.global_param][unitlib.units.global_param]\
            [unitlib.params.global_param.write_back_map] = write_back_map
            
        return

    def gen_units_map_json(self):
        units = {"units": self.units_map,
                 "hierarchy": self.hierarchy}
        with open("units.json","w") as file :
            json.dump(units, file)
        
        return
    
    def gen_topo_json(self):
        hierarchy = self.gen_hierarchy(False)
        
        topology = {"hierarchy": hierarchy,
                    # for debug
                    # "instances": self.instances,
                    # "unbinding": self.unbind_ports,
                    "binding"  : self.bindings}
        with open("topology.json", "w") as file :
            json.dump(topology, file)

        return
    
    def gen_binding_topo(self):
        # out port -> in port
        self.bind(
            unitlib.units.perfect_frontend, unitlib.ports.perfect_frontend.out_ports.fetch_backend_inst_out,
            unitlib.units.renaming_stage, unitlib.ports.renaming_stage.in_ports.preceding_renaming_inst_in
        )
        
        self.bind(
            unitlib.units.renaming_stage, unitlib.ports.renaming_stage.out_ports.renaming_following_inst_out,
            unitlib.units.dispatch_stage, unitlib.ports.dispatch_stage.in_ports.preceding_dispatch_inst_in
        )
        
        self.bind(
            unitlib.units.renaming_stage, unitlib.ports.renaming_stage.out_ports.renaming_following_inst_out,
            unitlib.units.rob, unitlib.ports.rob.in_ports.preceding_rob_inst_in
        )
        
        self.bind(
            unitlib.units.renaming_stage, unitlib.ports.renaming_stage.out_ports.renaming_preceding_credit_out,
            unitlib.units.perfect_frontend, unitlib.ports.perfect_frontend.in_ports.backend_fetch_credit_in
        )
        
        self.bind(
            unitlib.units.dispatch_stage, unitlib.ports.dispatch_stage.out_ports.dispatch_preceding_credit_out,
            unitlib.units.renaming_stage, unitlib.ports.renaming_stage.in_ports.following_renaming_credit_in
        )
        
        self.bind(
            unitlib.units.dispatch_stage, unitlib.ports.dispatch_stage.out_ports.dispatch_physical_reg_read_out,
            unitlib.units.physical_regfile, unitlib.ports.physical_regfile.in_ports.preceding_physical_regfile_read_in
        )  
        
        self.bind(
            unitlib.units.dispatch_stage, unitlib.ports.dispatch_stage.out_ports.dispatch_rs_inst_out,
            unitlib.units.reservation_station, unitlib.ports.reservation_station.in_ports.preceding_reservation_inst_in
        )  
        
        self.bind(
            unitlib.units.rob, unitlib.ports.rob.out_ports.rob_flush_out,
            unitlib.units.flush_manager, unitlib.ports.flush_manager.in_ports.rob_flush_manager_in
        )
        
        self.bind(
            unitlib.units.rob, unitlib.ports.rob.out_ports.rob_preceding_credit_out,
            unitlib.units.renaming_stage, unitlib.ports.renaming_stage.in_ports.rob_renaming_credit_in
        )
        
        self.bind(
            unitlib.units.rob, unitlib.ports.rob.out_ports.Rob_cmt_inst_out,
            unitlib.units.renaming_stage, unitlib.ports.renaming_stage.in_ports.Rob_cmt_inst_in
        )
        
        self.bind(
            unitlib.units.rob, unitlib.ports.rob.out_ports.rob_bpu_inst_out,
            unitlib.units.perfect_frontend, unitlib.ports.perfect_frontend.in_ports.backend_bpu_inst_in
        )
        
        self.bind(
            unitlib.units.rob, unitlib.ports.rob.out_ports.Rob_lsu_wakeup_out,
            unitlib.units.perfect_fu, unitlib.ports.perfect_fu.in_ports.preceding_rob_wakeup_store_in
        )
        
        self.bind(
            unitlib.units.rob, unitlib.ports.rob.out_ports.rob_redirect_pc_inst_out,
            unitlib.units.perfect_frontend, unitlib.ports.perfect_frontend.in_ports.backend_redirect_pc_inst_in
        )
        
        self.bind(
            unitlib.units.physical_regfile, unitlib.ports.physical_regfile.out_ports.physical_regfile_following_read_out,
            unitlib.units.dispatch_stage, unitlib.ports.dispatch_stage.in_ports.dispatch_physical_reg_read_in
        )
        
        self.bind(
            unitlib.units.reservation_station, unitlib.ports.reservation_station.out_ports.reservation_following_inst_out,
            unitlib.units.perfect_fu, unitlib.ports.perfect_fu.in_ports.preceding_func_inst_in
        )
        
        self.bind(
            unitlib.units.reservation_station, unitlib.ports.reservation_station.out_ports.reservation_preceding_credit_out,
            unitlib.units.dispatch_stage, unitlib.ports.dispatch_stage.in_ports.rs_dispatch_credit_in
        )
        
        self.bind(
            unitlib.units.perfect_fu, unitlib.ports.perfect_fu.out_ports.func_following_finish_out,
            unitlib.units.write_back_stage, unitlib.ports.write_back_stage.in_ports.preceding_write_back_inst_in
        )
        
        self.bind(
            unitlib.units.perfect_fu, unitlib.ports.perfect_fu.out_ports.func_rs_credit_out,
            unitlib.units.reservation_station, unitlib.ports.reservation_station.in_ports.following_reservation_credit_in
        )
        
        self.bind(
            unitlib.units.write_back_stage, unitlib.ports.write_back_stage.out_ports.write_back_following_port_out,
            unitlib.units.reservation_station, unitlib.ports.reservation_station.in_ports.forwarding_reservation_inst_in
        )
        
        self.bind(
            unitlib.units.write_back_stage, unitlib.ports.write_back_stage.out_ports.write_back_following_port_out,
            unitlib.units.dispatch_stage, unitlib.ports.dispatch_stage.in_ports.write_back_dispatch_port_in
        )
        
        self.bind(
            unitlib.units.write_back_stage, unitlib.ports.write_back_stage.out_ports.write_back_following_port_out,
            unitlib.units.rob, unitlib.ports.rob.in_ports.write_back_rob_finish_in
        )
        
        self.bind(
            unitlib.units.write_back_stage, unitlib.ports.write_back_stage.out_ports.write_back_following_port_out,
            unitlib.units.physical_regfile, unitlib.ports.physical_regfile.in_ports.preceding_physical_regfile_write_in
        )
        
        self.bind(
            unitlib.units.write_back_stage, unitlib.ports.write_back_stage.out_ports.preceding_write_back_credit_out,
            unitlib.units.perfect_fu, unitlib.ports.perfect_fu.in_ports.write_back_func_credit_in
        )
        
        self.bind(
            unitlib.units.flush_manager, unitlib.ports.flush_manager.out_ports.global_flush_signal_out,
            unitlib.units.perfect_fu, unitlib.ports.perfect_fu.in_ports.func_flush_in
        )
        
        self.bind(
            unitlib.units.flush_manager, unitlib.ports.flush_manager.out_ports.global_flush_signal_out,
            unitlib.units.dispatch_stage, unitlib.ports.dispatch_stage.in_ports.dispatch_flush_in
        )
        
        self.bind(
            unitlib.units.flush_manager, unitlib.ports.flush_manager.out_ports.global_flush_signal_out,
            unitlib.units.renaming_stage, unitlib.ports.renaming_stage.in_ports.renaming_flush_in
        )
        
        self.bind(
            unitlib.units.flush_manager, unitlib.ports.flush_manager.out_ports.global_flush_signal_out,
            unitlib.units.reservation_station, unitlib.ports.reservation_station.in_ports.reservation_flush_in
        )
        
        self.bind(
            unitlib.units.flush_manager, unitlib.ports.flush_manager.out_ports.global_flush_signal_out,
            unitlib.units.rob, unitlib.ports.rob.in_ports.rob_flush_in
        )
        
        self.bind(
            unitlib.units.flush_manager, unitlib.ports.flush_manager.out_ports.global_flush_signal_out,
            unitlib.units.write_back_stage, unitlib.ports.write_back_stage.in_ports.writeback_flush_in
        )
        
        return

    def bind(self, unit_out_name, port_out_name, unit_in_name, port_in_name):
        assert port_out_name in self.units_map[unit_out_name]["ports"]["out_ports"], f"in port not find in {unit_out_name}"
        assert port_in_name in self.units_map[unit_in_name]["ports"]["in_ports"], f"in port not find in {unit_in_name}"
        if port_out_name in self.unbind_ports:
            self.unbind_ports.remove(port_out_name)
        if port_in_name in self.unbind_ports:
            self.unbind_ports.remove(port_in_name)
        if len(self.instances[unit_out_name]) > 1 and len(self.instances[unit_in_name]) > 1:
            for i in range(len(self.instances[unit_out_name])):
                outport_name_list = list(self.instances[unit_out_name].keys())
                inport_name_list = list(self.instances[unit_in_name].keys())
                unit_out_hierarchy = self.units_map[unit_out_name]["hierarchy"]
                unit_in_hierarchy = self.units_map[unit_in_name]["hierarchy"]
                binding_outport_name = f"{unit_out_hierarchy}.{outport_name_list[i]}.ports.{port_out_name}"
                binding_inport_name = f"{unit_in_hierarchy}.{inport_name_list[i]}.ports.{port_in_name}"
                self.bindings.append({
                    "source": binding_outport_name,
                    "target": binding_inport_name
                })
        else:
            for unit_out_instance_name in self.instances[unit_out_name]:
                unit_out_hierarchy = self.units_map[unit_out_name]["hierarchy"]
                binding_outport_name = f"{unit_out_hierarchy}.{unit_out_instance_name}.ports.{port_out_name}"
                for unit_in_instance_name in self.instances[unit_in_name]:
                    unit_in_hierarchy = self.units_map[unit_in_name]["hierarchy"]
                    binding_inport_name = f"{unit_in_hierarchy}.{unit_in_instance_name}.ports.{port_in_name}"
                    self.bindings.append({
                        "source": binding_outport_name,
                        "target": binding_inport_name
                    })
        return 
    
    def gen_test(self):
        return        