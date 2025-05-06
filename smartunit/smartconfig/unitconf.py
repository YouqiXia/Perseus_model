import numpy as np

import json
import os

class UnitFactory(object):
    def __init__(self, module_name: str, core_count, cluster_count):
        self.module_name = module_name
        self.core_count = core_count
        self.cluster_count = cluster_count
        self.unit_instances = dict()
        self.unit_instance_params = dict()

    def create_unit(self, unit, nums, location = None):
        cluster_idx, core_idx = 0, 0
        if isinstance(location, int):
            core_idx = 0
        elif isinstance(location, tuple):
            cluster_idx, core_idx = location
        self.unit_instances[unit.name] = [nums, core_idx, cluster_idx]
        self.unit_instance_params[unit.name] = {}

    def set_params(self, unit, param_name, param_arr):
        if unit.name not in self.unit_instances:
            raise Exception('{} has not been create '.format(unit.name))
        if self.unit_instances[unit.name][0] != len(param_arr):
            raise Exception('{} instance num {} is different from len(param_arr) {}'.format(unit.name, self.unit_instances[unit.name][0], len(param_arr)))
        for i in range(0, len(param_arr)):
            param_arr[i] = str(param_arr[i])
        _, param_name = param_name.split('|')
        self.unit_instance_params[unit.name][param_name] = param_arr

    def merge(self, that):
        if not that:
            return self
        for key, val in that.unit_instances.items():
            if key in self.unit_instances:
                continue
            self.unit_instances[key] = val

        for key, val in that.unit_instance_params.items():
            if key in self.unit_instance_params:
                continue
            self.unit_instance_params[key] = val
        return self
    
    def print(self):
        print(self.unit_instances)
        print(self.unit_instance_params)

    def to_json(self, filepath):
        filepath = os.path.abspath(filepath)
        json_dict = {
            'unit_instances': self.unit_instances,
            'unit_instance_params': self.unit_instance_params
        }
        json_str = json.dumps(json_dict, indent=4)
        with open(filepath, 'w') as file:
            file.write(json_str)

class UnitBind(object):
    def __init__(self, module_name: str):
        self.module_name = module_name
        self.connections = dict()
        self.has_bind_set = set()

    def bind_unit(self, unit1, unit2, con_matrix):
        name1, name2 = unit1.name, unit2.name
        key = name1 + '|' + name2
        if not name1 < name2:
            key = name2 + '|' + name1
        if key in self.has_bind_set:
            raise Exception('{} and {} has binded!'.format(name1, name2))
        self.has_bind_set.add(key)
        self.connections[name1 + '|' + name2] = con_matrix

    def merge(self, that):
        if not that:
            return self
        for key, val in that.connections.items():
            if key in self.connections:
                continue
            self.connections[key] = val
        return self
    
    def print(self):
        print(self.connections)

    def to_json(self, filepath):
        filepath = os.path.abspath(filepath)
        json_dict = {
            'binds': self.connections,
        }
        json_str = json.dumps(json_dict, indent=4)
        with open(filepath, 'w') as file:
            file.write(json_str)
