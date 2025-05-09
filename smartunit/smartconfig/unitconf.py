import json
import os

def _get_location_key(location):
    cluster_idx, core_idx = 0, 0
    if isinstance(location, int):
        core_idx = location
    elif isinstance(location, tuple):
        cluster_idx, core_idx = location
    location_key = cluster_idx * 100 + core_idx
    return location_key

class UnitFactory(object):
    def __init__(self, module_name: str, core_count, cluster_count):
        self.module_name = module_name
        self.core_count = core_count
        self.cluster_count = cluster_count
        self.unit_instances = dict()

    def create_unit(self, unit, nums, location = None):
        location_key = _get_location_key(location)
        if location_key not in self.unit_instances:
            self.unit_instances[location_key] = dict()

        self.unit_instances[location_key][unit.name] = [nums, dict()]

    def set_params(self, param_name, param_arr, location = None):
        unitname, param_name = param_name.split('|')
        location_key = _get_location_key(location)
        if location_key not in self.unit_instances:
            raise Exception('No such location {}'.format(location_key))
        if unitname not in self.unit_instances[location_key]:
            raise Exception('{} has not been create in {}'.format(unitname), location_key)
        if self.unit_instances[location_key][unitname][0] != len(param_arr):
            raise Exception('{} instance num {} is different from len(param_arr) {}'.format(unitname, self.unit_instances[location_key][unitname][0], len(param_arr)))
        for i in range(0, len(param_arr)):
            param_arr[i] = str(param_arr[i])
        self.unit_instances[location_key][unitname][1][param_name] = param_arr

    def merge(self, that):
        if not that:
            return self
        for key, subdict1 in that.unit_instances.items():
            if key not in self.unit_instances:
                self.unit_instances[key] = dict()
            for unitkey in subdict1:
                self.unit_instances[key][unitkey] = subdict1[unitkey]
        return self
    
    def print(self):
        print(self.unit_instances)

    def to_json(self, filepath):
        filepath = os.path.abspath(filepath)
        json_str = json.dumps(self.unit_instances, indent=4)
        with open(filepath, 'w') as file:
            file.write(json_str)

class UnitBind(object):
    def __init__(self, module_name: str):
        self.module_name = module_name
        self.binds = dict()
        self.has_bind_set = set()

    def bind_unit(self, unit1, unit2, bind_matrix, location1=None, location2=None):
        location1_key, location2_key = _get_location_key(location1), _get_location_key(location2)
        name1, name2 = unit1.name, unit2.name
        key = '{}|{}|{}|{}'.format(name1, location1_key, name2, location2_key)
        if not name1 < name2:
            key = '{}|{}|{}|{}'.format(name2, location2_key, name1, location1_key)
        if key in self.has_bind_set:
            raise Exception('{} and {} has binded!'.format(name1, name2))
        self.has_bind_set.add(key)
        self.binds['{}|{}|{}|{}'.format(name1, location1_key, name2, location2_key)] = bind_matrix

    def merge(self, that):
        if not that:
            return self
        for key, val in that.binds.items():
            self.binds[key] = val
        return self
    
    def print(self):
        print(self.binds)

    def to_json(self, filepath):
        filepath = os.path.abspath(filepath)
        json_str = json.dumps(self.binds, indent=4)
        with open(filepath, 'w') as file:
            file.write(json_str)
