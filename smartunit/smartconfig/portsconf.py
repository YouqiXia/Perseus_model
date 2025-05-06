import os
import json

class PortsConf(object):
    def __init__(self, module_name: str):
        self.module_name = module_name
        self.bindings = {}

    def bind(self, outport, inport):
        unit1, outname = outport.split('|')
        unit2, inname = inport.split('|')
        key = unit1 + '|' + unit2
        if unit1 > unit2:
            unit1, unit2 = unit2, unit1
            key = unit1 + '|' + unit2
        if key not in self.bindings:
            self.bindings[key] = []
        self.bindings[key].append(outname)
        self.bindings[key].append(inname)

    def get_bindding(self, key):
        if key not in self.bindings:
            return None
        return self.bindings[key]
    
    def merge(self, that):
        if not that:
            return self
        for key, binding in that.bindings.items():
            if key in self.bindings:
                continue
            self.bindings[key] = binding
        return self
    
    def print(self):
        for key, value in self.bindings.items():
            print('---Connection: {}---\n'.format(key), end="")
            for binding in value:
                print('out: {}, in: {}\n'.format(binding[0], binding[1]), end="")

    def to_json(self, filepath):
        filepath = os.path.abspath(filepath)
        json_dict = {
            'bindings': self.bindings,
        }
        json_str = json.dumps(json_dict, indent=4)
        with open(filepath, 'w') as file:
            file.write(json_str)
        