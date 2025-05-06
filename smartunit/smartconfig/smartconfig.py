#!/usr/bin/python3

import os
import sys
import argparse
import importlib
import json
import shutil

import string

_script_dir = os.path.dirname(os.path.abspath(__file__))
_pytemplate_file = _script_dir + '/module_tpt.py.tpl'
_module_level_file = _script_dir + '/intermodule.json'
_frame_file = _script_dir + '/frameconf.py.tpl'

class GenTemplate(object):
    def __init__(self):
        self._module_level_dict = None

    def load_module_level(self):
        with open(_module_level_file) as file:
            self._module_level_dict = json.load(file)

    def gen_template(self):
        self.load_module_level()

        # Read py template
        with open(_pytemplate_file) as file:
            pytemplate_str = file.read()
        
        current_dir = os.getcwd()
        pydir = current_dir + '/pyconf'
        if (os.path.exists(pydir)):
            raise Exception('{} exists!'.format(pydir))
        os.mkdir(pydir)

        # import UnitLib
        unitmodule = importlib.import_module('UnitLib')
        
        # Loop all sub-module in UnitLib
        for module_name in dir(unitmodule):
            if module_name.startswith('__'):
                continue
            # Generate py file for each submodule
            template_obj = string.Template(pytemplate_str)
            # extra_content = self.build_extra_content(module_name)
            substitutions = {
                'extra_content': 'import UnitLib.{} as {}'.format(module_name, module_name),
                'name': '\'{}\''.format(module_name)
            }
            pyinstance_str = template_obj.substitute(substitutions)

            with open('{}/{}.py'.format(pydir, module_name), 'w') as file:
                file.write(pyinstance_str)

        if self._module_level_dict:
            for str in self._module_level_dict['intermodule']:
                template_obj = string.Template(pytemplate_str)
                modules = str.split('-')
                content = ''
                for module in modules:
                    content += 'import UnitLib.{} as {}\n'.format(module, module)
                substitutions = {
                    'extra_content': '{}'.format(content),
                    'name': '\'{}\''.format(str)
                }
                pyinstance_str = template_obj.substitute(substitutions)

                with open('{}/{}.py'.format(pydir, str), 'w') as file:
                    file.write(pyinstance_str)
        
        shutil.copy(_frame_file, pydir + '/frameconf.py')

class GenModelConf(object):
    def __init__(self):
        self._module_level_dict = None
        self.unit_conf = None
        self.unit_bind = None
        self.ports_conf = None
        self.default_params = None

    def load_module_level(self):
        with open(_module_level_file) as file:
            self._module_level_dict = json.load(file)

    def gen_model_config(self, pyconf: str):
        pyconf = os.path.abspath(pyconf)
        print(pyconf)
        sys.path.append(pyconf)
        self.load_module_level()

        # import UnitLib
        unitmodule = importlib.import_module('UnitLib')
        
        # Loop all sub-module in UnitLib
        for module_name in dir(unitmodule):
            if module_name.startswith('__'):
                continue
            submodule = importlib.import_module(module_name)
            submodule.build_units()
            submodule.build_ports_conf()
            submodule.bind_units()
            submodule.setup_dparams()
            self.unit_conf = submodule.unit_conf.merge(self.unit_conf)
            self.ports_conf = submodule.ports_conf.merge(self.ports_conf)
            self.unit_bind = submodule.unit_bind.merge(self.unit_bind)
            self.default_params = submodule.default_params.merge(self.default_params)

        if self._module_level_dict:
            for str in self._module_level_dict['intermodule']:
                submodule = importlib.import_module(str) 
                submodule.build_units()
                submodule.build_ports_conf()
                submodule.bind_units()
                submodule.setup_dparams()
                self.unit_conf = submodule.unit_conf.merge(self.unit_conf)
                self.ports_conf = submodule.ports_conf.merge(self.ports_conf)
                self.unit_bind = submodule.unit_bind.merge(self.unit_bind)
                self.default_params = submodule.default_params.merge(self.default_params)

        self.unit_conf.to_json(os.getcwd() + '/unit_conf.json')
        self.ports_conf.to_json(os.getcwd() + '/port_conf.json')
        self.unit_bind.to_json(os.getcwd() + '/unit_bind.json')
        self.default_params.to_json(os.getcwd() + '/default_params.json')

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='cmd parser')
    parser.add_argument('-p', '--pyconf', type=str, help='')
    parser.add_argument('-u', '--unitlib', type=str, help='')

    args = parser.parse_args()

    if args.unitlib is not None:
        sys.path.append(args.unitlib)

    if args.pyconf is not None:
        # Generate model conf
        GenModelConf().gen_model_config(args.pyconf)
    else:
        # Generate template
        GenTemplate().gen_template()
    