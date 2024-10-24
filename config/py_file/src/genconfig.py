import unitlib
import json
import os
import subprocess

from simple_arch.simple_arch_factory import SimpleConfigFactory as Factory

class Config:
    def __init__(self, units_, factory_):
        self.units = units_
        self.factory = factory_
        self._gen_arch(factory_)
        pass
    
    def _gen_arch(self, factory): # use factory to reduce code modification
        self.config_units = factory.gen_unit_config(units = self.units)
        
        self.config_arch = factory.gen_arch_config()
        self.config_instances = \
            factory.gen_instance_config(config_units = self.config_units,
                                        config_arch = self.config_arch)
        
        self.config_params = factory.gen_param_config()
        self.config_params.gen_params(hierarchy = self.config_instances.get_hierarchy(), 
                                      instances = self.config_instances.get_instances(),
                                      arch_config = self.config_arch)
        
        self.config_ports = \
            factory.gen_port_config(units = self.units, 
                                    units_map = self.config_units.get_units(), 
                                    instances = self.config_instances.get_instances())
        return
    
    def gen_units_map_json(self):
        units = {"units": self.config_units.get_units(),
                 "hierarchy": self.config_units.get_hierarchy()}
        with open("../units.json","w") as file :
            json.dump(units, file)
        
        return
    
    def gen_demo_topo_json(self):
        self.config_params.gen_params(self.config_instances.get_hierarchy(), 
                                      self.config_instances.get_instances(),
                                      self.config_arch)
        topology = {"hierarchy": self.config_instances.get_hierarchy(),
                    # for debug
                    # "instances": self.config_instances.get_instances(),
                    # "unbinding": self.config_ports.get_unbinding_ports(),
                    #
                    "binding"  : self.config_ports.get_binding_topo()}
        with open("../topology.json", "w") as file :
            json.dump(topology, file)

        return
    
    def gen_topo_json(self):
        self.config_params.gen_params(self.config_instances.get_hierarchy(), 
                                      self.config_instances.get_instances(),
                                      self.config_arch)
        topology = {"hierarchy": self.config_instances.get_hierarchy(),
                    # for debug
                    # "instances": self.config_instances.get_instances(),
                    # "unbinding": self.config_ports.get_unbinding_ports(),
                    #
                    "binding"  : self.config_ports.get_binding_topo()}
        folder_name = f"../topo/{self.config_arch.get_arch_name()}"
        if not os.path.exists(folder_name):
            os.makedirs(folder_name)
        with open(f"{folder_name}/topology.json", "w") as file :
            json.dump(topology, file)

        return
    
    def gen_all_topo_json(self):
        hierarchy_map = self.config_params\
            .gen_all_exploration_params(self.config_instances.get_hierarchy(),
                                        self.config_instances.get_instances(),
                                        self.config_arch)
        folder_name = f"../topo/{self.config_arch.get_arch_name()}"
        if not os.path.exists(folder_name):
            os.makedirs(folder_name)
        for file_name, hierarchy in hierarchy_map.items():
            topology = {"hierarchy": hierarchy,
                        "binding"  : self.config_ports.get_binding_topo()}
            with open(f"{folder_name}/{file_name}.json", "w") as file :
                json.dump(topology, file)

def test():
    model = "../../../cmake-build-debug/model"
    json_file = "../topology.json"
    trace_file = "../../../traces/dhry_riscv.zstf"
    command = f"{model} --json {json_file} --workload {trace_file}"
    subprocess.run(command, shell=True)

if __name__ == "__main__":
    units = unitlib.UnitsSet()
    factory = Factory() 
    config = Config(units_ = units,
                    factory_ = factory)
    config.gen_units_map_json()
    config.gen_demo_topo_json()
    config.gen_topo_json()
    config.gen_all_topo_json()
    test()











            
        