import unitlib
import base_arch.port_config as port_config
import numpy as np

class PortConfig(port_config.PortConfig):
    def _gen_binding_map(self):
        # out - in
        # unitlib.units.scheduler - unitlib.units.perfect_fu
        row = len(self.instances[unitlib.units.scheduler])
        column = len(self.instances[unitlib.units.perfect_fu])
        matrix = np.zeros((row, column), dtype=bool)
        for i in range(row):
            for j in range(column):
                if i == j:
                    matrix[i][j] = True
        self.binding_map.add_matrix(
            unitlib.units.scheduler,
            unitlib.units.perfect_fu,
            matrix.copy()
        )
        return 
    
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
            unitlib.units.renaming_stage, unitlib.ports.renaming_stage.out_ports.renaming_check_busy_out,
            unitlib.units.busy_table, unitlib.ports.busy_table.in_ports.check_busy_in
        )
        
        self._bind(
            unitlib.units.dispatch_stage, unitlib.ports.dispatch_stage.out_ports.dispatch_preceding_credit_out,
            unitlib.units.renaming_stage, unitlib.ports.renaming_stage.in_ports.following_renaming_credit_in
        )
        
        self._bind(
            unitlib.units.dispatch_stage, unitlib.ports.dispatch_stage.out_ports.dispatch_rs_inst_out,
            unitlib.units.scheduler, unitlib.ports.scheduler.in_ports.preceding_scheduler_inst_in
        )
        
        self._bind(
            unitlib.units.dispatch_stage, unitlib.ports.dispatch_stage.out_ports.dispatch_rs_inst_out,
            unitlib.units.spec_busy_table, unitlib.ports.spec_busy_table.in_ports.check_in
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
            unitlib.units.scheduler, unitlib.ports.scheduler.out_ports.scheduler_preceding_credit_out,
            unitlib.units.dispatch_stage, unitlib.ports.dispatch_stage.in_ports.rs_dispatch_credit_in
        )
        
        self._bind(
            unitlib.units.scheduler, unitlib.ports.scheduler.out_ports.scheduler_following_inst_out,
            unitlib.units.physical_regfile, unitlib.ports.physical_regfile.in_ports.preceding_physical_regfile_read_in
        )  
        
        self._bind(
            unitlib.units.scheduler, unitlib.ports.scheduler.out_ports.spec_wake_up_out,
            unitlib.units.scheduler, unitlib.ports.scheduler.in_ports.spec_wake_up_in
        )
        
        self._bind(
            unitlib.units.scheduler, unitlib.ports.scheduler.out_ports.spec_wake_up_out,
            unitlib.units.spec_busy_table, unitlib.ports.spec_busy_table.in_ports.wakeup_in
        )
        
        self._bind(
            unitlib.units.physical_regfile, unitlib.ports.physical_regfile.out_ports.physical_regfile_following_read_out,
            unitlib.units.staging_buffer, unitlib.ports.staging_buffer.in_ports.preceding_inst_in
        )
        
        self._bind(
            unitlib.units.physical_regfile, unitlib.ports.physical_regfile.out_ports.preceding_credit_out,
            unitlib.units.scheduler, unitlib.ports.scheduler.in_ports.following_scheduler_credit_in
        )
        
        self._bind(
            unitlib.units.staging_buffer, unitlib.ports.staging_buffer.out_ports.following_inst_out,
            unitlib.units.perfect_fu, unitlib.ports.perfect_fu.in_ports.preceding_func_inst_in
        )
        
        self._bind(
            unitlib.units.staging_buffer, unitlib.ports.staging_buffer.out_ports.preceding_credit_out,
            unitlib.units.physical_regfile, unitlib.ports.physical_regfile.in_ports.following_credit_in
        )
        
        self._bind(
            unitlib.units.staging_buffer, unitlib.ports.staging_buffer.out_ports.wakeup_resolve_inst_out,
            unitlib.units.scheduler, unitlib.ports.scheduler.in_ports.wakeup_resolve_in
        )
        
        self._bind(
            unitlib.units.staging_buffer, unitlib.ports.staging_buffer.out_ports.wakeup_resolve_inst_out,
            unitlib.units.spec_busy_table, unitlib.ports.spec_busy_table.in_ports.wakeup_resolve_in
        )
        
        self._bind(
            unitlib.units.perfect_fu, unitlib.ports.perfect_fu.out_ports.following_write_back_out,
            unitlib.units.write_back_stage, unitlib.ports.write_back_stage.in_ports.preceding_write_back_inst_in
        )
        
        self._bind(
            unitlib.units.perfect_fu, unitlib.ports.perfect_fu.out_ports.following_write_back_out,
            unitlib.units.physical_regfile, unitlib.ports.physical_regfile.in_ports.bypass_inst_in
        )
        
        self._bind(
            unitlib.units.perfect_fu, unitlib.ports.perfect_fu.out_ports.following_write_back_out,
            unitlib.units.staging_buffer, unitlib.ports.staging_buffer.in_ports.bypass_inst_in
        )
        
        self._bind(
            unitlib.units.perfect_fu, unitlib.ports.perfect_fu.out_ports.func_rs_credit_out,
            unitlib.units.staging_buffer, unitlib.ports.staging_buffer.in_ports.following_credit_in
        )
        
        self._bind(
            unitlib.units.perfect_fu, unitlib.ports.perfect_fu.out_ports.following_rob_finish_out,
            unitlib.units.rob, unitlib.ports.rob.in_ports.write_back_rob_finish_in
        )
        
        self._bind(
            unitlib.units.write_back_stage, unitlib.ports.write_back_stage.out_ports.write_back_following_port_out,
            unitlib.units.scheduler, unitlib.ports.scheduler.in_ports.forwarding_scheduler_inst_in
        )
        
        self._bind(
            unitlib.units.write_back_stage, unitlib.ports.write_back_stage.out_ports.write_back_following_port_out,
            unitlib.units.dispatch_stage, unitlib.ports.dispatch_stage.in_ports.write_back_dispatch_port_in
        )
        
        self._bind(
            unitlib.units.write_back_stage, unitlib.ports.write_back_stage.out_ports.write_back_following_port_out,
            unitlib.units.physical_regfile, unitlib.ports.physical_regfile.in_ports.preceding_physical_regfile_write_in
        )
        
        self._bind(
            unitlib.units.write_back_stage, unitlib.ports.write_back_stage.out_ports.write_back_following_port_out,
            unitlib.units.busy_table, unitlib.ports.busy_table.in_ports.update_busy_in
        )
        
        self._bind(
            unitlib.units.write_back_stage, unitlib.ports.write_back_stage.out_ports.write_back_following_port_out,
            unitlib.units.spec_busy_table, unitlib.ports.spec_busy_table.in_ports.update_in
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
            unitlib.units.scheduler, unitlib.ports.scheduler.in_ports.scheduler_flush_in
        )
        
        self._bind(
            unitlib.units.flush_manager, unitlib.ports.flush_manager.out_ports.global_flush_signal_out,
            unitlib.units.rob, unitlib.ports.rob.in_ports.rob_flush_in
        )
        
        self._bind(
            unitlib.units.flush_manager, unitlib.ports.flush_manager.out_ports.global_flush_signal_out,
            unitlib.units.write_back_stage, unitlib.ports.write_back_stage.in_ports.writeback_flush_in
        )
        
        self._bind(
            unitlib.units.flush_manager, unitlib.ports.flush_manager.out_ports.global_flush_signal_out,
            unitlib.units.busy_table, unitlib.ports.busy_table.in_ports.busy_table_flush_in
        )
        
        self._bind(
            unitlib.units.flush_manager, unitlib.ports.flush_manager.out_ports.global_flush_signal_out,
            unitlib.units.spec_busy_table, unitlib.ports.spec_busy_table.in_ports.flush_in
        )
        
        return self.bindings

    def _bind(self, unit_out_name, port_out_name, unit_in_name, port_in_name):
        assert port_out_name in self.units_map[unit_out_name]["ports"]["out_ports"], f"in port not find in {unit_out_name}"
        assert port_in_name in self.units_map[unit_in_name]["ports"]["in_ports"], f"in port not find in {unit_in_name}"
        
        unit_out_hierarchy = self.units_map[unit_out_name]["hierarchy"]
        unit_in_hierarchy = self.units_map[unit_in_name]["hierarchy"]
        
        if port_out_name in self.unbinding_ports:
            self.unbinding_ports.remove(port_out_name)
        if port_in_name in self.unbinding_ports:
            self.unbinding_ports.remove(port_in_name)
        
        if not self.binding_map.isMatrix(unit_out_name, unit_in_name):
            for unit_out_instance_name in self.instances[unit_out_name]:
                binding_outport_name = f"{unit_out_hierarchy}.{unit_out_instance_name}.ports.{port_out_name}"
                for unit_in_instance_name in self.instances[unit_in_name]:
                    binding_inport_name = f"{unit_in_hierarchy}.{unit_in_instance_name}.ports.{port_in_name}"
                    self.bindings.append({
                        "source": binding_outport_name,
                        "target": binding_inport_name
                    })
        else:
            matrix = self.binding_map.get_matrix(unit_out_name, unit_in_name)
            for i in range(matrix.shape[0]):
                for j in range(matrix.shape[1]):
                    if matrix[i][j]:
                        unit_out_instance_name = self.instances["instance_topo"]["table"][unit_out_name][i]
                        unit_in_instance_name = self.instances["instance_topo"]["table"][unit_in_name][j]
                        binding_outport_name = f"{unit_out_hierarchy}.{unit_out_instance_name}.ports.{port_out_name}"
                        binding_inport_name = f"{unit_in_hierarchy}.{unit_in_instance_name}.ports.{port_in_name}"
                        self.bindings.append({
                            "source": binding_outport_name,
                            "target": binding_inport_name
                        })
        return 