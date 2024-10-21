class PortConfig:
    def __init__(self, units_, units_map_, instances_):
        # for read only copy
        self.units_map = units_map_
        self.instances = instances_
        
        # real generation for ports
        self.bindings = []
        self.unbinding_ports = self._gen_ports(units_.GetUnitsInfo())
        self._gen_binding_topo()
        return
    
    def get_binding_topo(self):
        return self.bindings
    
    def get_unbinding_ports(self):
        return self.unbinding_ports
    
    def _gen_ports(self, units):
        ports_list = []
        for unit_name, unit_statistics in units.items() :
            for out_direction, ports in unit_statistics.ports_info.items():
                ports_list += ports
            
        return ports_list
    
    def _gen_binding_topo(self):
        return self.bindings

    def _bind(self, unit_out_name, port_out_name, unit_in_name, port_in_name):
        return 