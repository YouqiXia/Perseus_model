import os
import json

class PortsConf(object):
    def __init__(self, module_name: str):
        self.module_name = module_name
        self.bindings = {}

    def bind(self, outport, inport):
        unit1, portname1 = outport.split('|')
        unit2, portname2 = inport.split('|')
        key = unit1 + '|' + unit2
        if unit1 > unit2:
            unit1, unit2 = unit2, unit1
            portname1, portname2 = portname2, portname1
            key = unit1 + '|' + unit2
        if key not in self.bindings:
            self.bindings[key] = []
        self.bindings[key].append(portname1)
        self.bindings[key].append(portname2)

    def get_bindding(self, key):
        if key not in self.bindings:
            return None
        return self.bindings[key]
    
    def merge(self, that):
        if not that:
            return self
        for key, binding in that.bindings.items():
            self.bindings[key] = binding
        return self
    
    def print(self):
        for key, value in self.bindings.items():
            print('---Connection: {}---\n'.format(key), end="")
            for binding in value:
                print('out: {}, in: {}\n'.format(binding[0], binding[1]), end="")

    def to_json(self, filepath):
        filepath = os.path.abspath(filepath)
        json_str = json.dumps(self.bindings, indent=4)
        with open(filepath, 'w') as file:
            file.write(json_str)
        