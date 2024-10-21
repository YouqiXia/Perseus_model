import unitlib
import base_arch.port_config as port_config

class SimplePortConfig(port_config.PortConfig):
    def _gen_binding_topo(self):
        # out port -> in port
        self._bind(
            unitlib.units.perfect_frontend, unitlib.ports.perfect_frontend.out_ports.fetch_backend_inst_out,
            unitlib.units.renaming_stage, unitlib.ports.renaming_stage.in_ports.preceding_renaming_inst_in
        )
        
        self._bind(
            unitlib.units.renaming_stage, unitlib.ports.renaming_stage.out_ports.renaming_following_inst_out,
            unitlib.units.dispatch_stage, unitlib.ports.dispatch_stage.in_ports.preceding_dispatch_inst_in
        )
        
        self._bind(
            unitlib.units.renaming_stage, unitlib.ports.renaming_stage.out_ports.renaming_following_inst_out,
            unitlib.units.rob, unitlib.ports.rob.in_ports.preceding_rob_inst_in
        )
        
        self._bind(
            unitlib.units.renaming_stage, unitlib.ports.renaming_stage.out_ports.renaming_preceding_credit_out,
            unitlib.units.perfect_frontend, unitlib.ports.perfect_frontend.in_ports.backend_fetch_credit_in
        )
        
        self._bind(
            unitlib.units.dispatch_stage, unitlib.ports.dispatch_stage.out_ports.dispatch_preceding_credit_out,
            unitlib.units.renaming_stage, unitlib.ports.renaming_stage.in_ports.following_renaming_credit_in
        )
        
        self._bind(
            unitlib.units.dispatch_stage, unitlib.ports.dispatch_stage.out_ports.dispatch_physical_reg_read_out,
            unitlib.units.physical_regfile, unitlib.ports.physical_regfile.in_ports.preceding_physical_regfile_read_in
        )  
        
        self._bind(
            unitlib.units.dispatch_stage, unitlib.ports.dispatch_stage.out_ports.dispatch_rs_inst_out,
            unitlib.units.reservation_station, unitlib.ports.reservation_station.in_ports.preceding_reservation_inst_in
        )  
        
        self._bind(
            unitlib.units.rob, unitlib.ports.rob.out_ports.rob_flush_out,
            unitlib.units.flush_manager, unitlib.ports.flush_manager.in_ports.rob_flush_manager_in
        )
        
        self._bind(
            unitlib.units.rob, unitlib.ports.rob.out_ports.rob_preceding_credit_out,
            unitlib.units.renaming_stage, unitlib.ports.renaming_stage.in_ports.rob_renaming_credit_in
        )
        
        self._bind(
            unitlib.units.rob, unitlib.ports.rob.out_ports.Rob_cmt_inst_out,
            unitlib.units.renaming_stage, unitlib.ports.renaming_stage.in_ports.Rob_cmt_inst_in
        )
        
        self._bind(
            unitlib.units.rob, unitlib.ports.rob.out_ports.rob_bpu_inst_out,
            unitlib.units.perfect_frontend, unitlib.ports.perfect_frontend.in_ports.backend_bpu_inst_in
        )
        
        self._bind(
            unitlib.units.rob, unitlib.ports.rob.out_ports.Rob_lsu_wakeup_out,
            unitlib.units.perfect_fu, unitlib.ports.perfect_fu.in_ports.preceding_rob_wakeup_store_in
        )
        
        self._bind(
            unitlib.units.rob, unitlib.ports.rob.out_ports.rob_redirect_pc_inst_out,
            unitlib.units.perfect_frontend, unitlib.ports.perfect_frontend.in_ports.backend_redirect_pc_inst_in
        )
        
        self._bind(
            unitlib.units.physical_regfile, unitlib.ports.physical_regfile.out_ports.physical_regfile_following_read_out,
            unitlib.units.dispatch_stage, unitlib.ports.dispatch_stage.in_ports.dispatch_physical_reg_read_in
        )
        
        self._bind(
            unitlib.units.reservation_station, unitlib.ports.reservation_station.out_ports.reservation_following_inst_out,
            unitlib.units.perfect_fu, unitlib.ports.perfect_fu.in_ports.preceding_func_inst_in
        )
        
        self._bind(
            unitlib.units.reservation_station, unitlib.ports.reservation_station.out_ports.reservation_preceding_credit_out,
            unitlib.units.dispatch_stage, unitlib.ports.dispatch_stage.in_ports.rs_dispatch_credit_in
        )
        
        self._bind(
            unitlib.units.perfect_fu, unitlib.ports.perfect_fu.out_ports.func_following_finish_out,
            unitlib.units.write_back_stage, unitlib.ports.write_back_stage.in_ports.preceding_write_back_inst_in
        )
        
        self._bind(
            unitlib.units.perfect_fu, unitlib.ports.perfect_fu.out_ports.func_rs_credit_out,
            unitlib.units.reservation_station, unitlib.ports.reservation_station.in_ports.following_reservation_credit_in
        )
        
        self._bind(
            unitlib.units.write_back_stage, unitlib.ports.write_back_stage.out_ports.write_back_following_port_out,
            unitlib.units.reservation_station, unitlib.ports.reservation_station.in_ports.forwarding_reservation_inst_in
        )
        
        self._bind(
            unitlib.units.write_back_stage, unitlib.ports.write_back_stage.out_ports.write_back_following_port_out,
            unitlib.units.dispatch_stage, unitlib.ports.dispatch_stage.in_ports.write_back_dispatch_port_in
        )
        
        self._bind(
            unitlib.units.write_back_stage, unitlib.ports.write_back_stage.out_ports.write_back_following_port_out,
            unitlib.units.rob, unitlib.ports.rob.in_ports.write_back_rob_finish_in
        )
        
        self._bind(
            unitlib.units.write_back_stage, unitlib.ports.write_back_stage.out_ports.write_back_following_port_out,
            unitlib.units.physical_regfile, unitlib.ports.physical_regfile.in_ports.preceding_physical_regfile_write_in
        )
        
        self._bind(
            unitlib.units.write_back_stage, unitlib.ports.write_back_stage.out_ports.preceding_write_back_credit_out,
            unitlib.units.perfect_fu, unitlib.ports.perfect_fu.in_ports.write_back_func_credit_in
        )
        
        self._bind(
            unitlib.units.flush_manager, unitlib.ports.flush_manager.out_ports.global_flush_signal_out,
            unitlib.units.perfect_fu, unitlib.ports.perfect_fu.in_ports.func_flush_in
        )
        
        self._bind(
            unitlib.units.flush_manager, unitlib.ports.flush_manager.out_ports.global_flush_signal_out,
            unitlib.units.dispatch_stage, unitlib.ports.dispatch_stage.in_ports.dispatch_flush_in
        )
        
        self._bind(
            unitlib.units.flush_manager, unitlib.ports.flush_manager.out_ports.global_flush_signal_out,
            unitlib.units.renaming_stage, unitlib.ports.renaming_stage.in_ports.renaming_flush_in
        )
        
        self._bind(
            unitlib.units.flush_manager, unitlib.ports.flush_manager.out_ports.global_flush_signal_out,
            unitlib.units.reservation_station, unitlib.ports.reservation_station.in_ports.reservation_flush_in
        )
        
        self._bind(
            unitlib.units.flush_manager, unitlib.ports.flush_manager.out_ports.global_flush_signal_out,
            unitlib.units.rob, unitlib.ports.rob.in_ports.rob_flush_in
        )
        
        self._bind(
            unitlib.units.flush_manager, unitlib.ports.flush_manager.out_ports.global_flush_signal_out,
            unitlib.units.write_back_stage, unitlib.ports.write_back_stage.in_ports.writeback_flush_in
        )
        
        return self.bindings

    def _bind(self, unit_out_name, port_out_name, unit_in_name, port_in_name):
        assert port_out_name in self.units_map[unit_out_name]["ports"]["out_ports"], f"in port not find in {unit_out_name}"
        assert port_in_name in self.units_map[unit_in_name]["ports"]["in_ports"], f"in port not find in {unit_in_name}"
        if port_out_name in self.unbinding_ports:
            self.unbinding_ports.remove(port_out_name)
        if port_in_name in self.unbinding_ports:
            self.unbinding_ports.remove(port_in_name)
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