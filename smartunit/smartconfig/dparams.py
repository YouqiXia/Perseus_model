import json
import os

class DParams(object):
    def __init__(self, module_name: str):
        self.module_name = module_name
        self.params_dict: dict = {}

    def set_param(self, key, value):
        unitkey, param_key = key.split('|')
        if unitkey not in self.params_dict:
            self.params_dict[unitkey] = {}
        self.params_dict[unitkey][param_key] = str(value)

    def merge(self, that):
        if not that:
            return self
        for unitkey, subdict in that.params_dict.items():
            for param_key, val in subdict.items():
                if unitkey not in self.params_dict:
                    self.params_dict[unitkey] = {}
                self.params_dict[unitkey][param_key] = val
        return self

    def to_json(self, filepath):
        filepath = os.path.abspath(filepath)
        json_str = json.dumps(self.params_dict, indent=4)
        with open(filepath, 'w') as file:
            file.write(json_str)

    def print(self):
        print(self.params_dict)