import os
import json

def _get_location_key(location):
    cluster_idx, core_idx = 0, 0
    if isinstance(location, int):
        core_idx = location
    elif isinstance(location, tuple):
        cluster_idx, core_idx = location
    location_key = cluster_idx * 100 + core_idx
    return location_key

class PortsBind(object):
    def __init__(self, module_name: str):
        self.module_name = module_name
        self.bindings = dict()
        self.has_bind_set = set()

    def bind(self, port1, port2, bind_matrix, location1=None, location2=None):
        location1_key, location2_key = _get_location_key(location1), _get_location_key(location2)
        name1, name2 = port1, port2
        key = '{}|{}|{}|{}'.format(name1, location1_key, name2, location2_key)
        if not name1 < name2:
            key = '{}|{}|{}|{}'.format(name2, location2_key, name1, location1_key)
        if key in self.has_bind_set:
            raise Exception('{} and {} has binded!'.format(name1, name2))
        self.has_bind_set.add(key)
        self.bindings['{}|{}|{}|{}'.format(name1, location1_key, name2, location2_key)] = bind_matrix

    def merge(self, that):
        if not that:
            return self
        for key, val in that.bindings.items():
            self.bindings[key] = val
        return self
    
    def print(self):
        print(self.bindings)

    def to_json(self, filepath):
        filepath = os.path.abspath(filepath)
        json_str = json.dumps(self.bindings, indent=4)
        with open(filepath, 'w') as file:
            file.write(json_str)