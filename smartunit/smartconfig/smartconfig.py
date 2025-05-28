#!/usr/bin/env python3

import os
import sys
import argparse
import importlib
import json
import shutil

import string

_script_dir = os.path.dirname(os.path.abspath(__file__))
_pytemplate_file = _script_dir + '/module_tpt.py.tpl'
_frame_file = _script_dir + '/frameconf.py.tpl'

class GenTemplate(object):
    def __init__(self):
        self._module_level_dict = None

    def load_module_level(self, module_level_file):
        with open(module_level_file) as file:
            self._module_level_dict = json.load(file)

    def gen_template(self, module_level_file):
        self.load_module_level(module_level_file)

        # Read py template
        with open(_pytemplate_file) as file:
            pytemplate_str = file.read()
        
        current_dir = os.getcwd()
        pydir = current_dir + '/pyconf'
        if (os.path.exists(pydir)):
            user_input = input('Remove existing {}? y/[n]. '.format(pydir))
            if user_input == 'y':
                shutil.rmtree(pydir)
            else:
                raise Exception('{} exists!'.format(pydir))
        os.mkdir(pydir)

        # import UnitLib
        unitmodule = importlib.import_module('UnitLib')

        # submodules string
        submodules_str = ''
        
        # Loop all sub-module in UnitLib
        for module_name in dir(unitmodule):
            if module_name.startswith('__'):
                continue

            # Add to submodules_str
            submodules_str += '\'{}\''.format(module_name) if len(submodules_str) == 0 else ', \'{}\''.format(module_name)

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
                # Add to submodules_str
                submodules_str += '\'{}\''.format(str) if len(submodules_str) == 0 else ', \'{}\''.format(str)

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
        
        # Read frameconf_tpl
        with open(_frame_file) as file:
            frameconf_tpl = file.read()
        substitutions = {
            'submodules': submodules_str
        }
        template_obj = string.Template(frameconf_tpl)
        with open('{}/{}'.format(pydir, 'frameconf.py'), 'w') as file:
            file.write(template_obj.substitute(substitutions))

class GenModelConf(object):
    def __init__(self):
        self._module_level_dict = None
        self.unit_conf = None
        self.ports_bind = None
        self.default_params = None

    def gen_model_config(self, pyconf: str):
        pyconf = os.path.abspath(pyconf)
        print(pyconf)
        sys.path.append(pyconf)

        # import frameconf
        frameconf_mod = importlib.import_module('frameconf')
        
        # Loop all sub-module in frameconf
        for module_name in frameconf_mod.SUBMODULES:
            submodule = importlib.import_module(module_name)
            submodule.build_units()
            submodule.bind_ports()
            submodule.setup_dparams()
            self.unit_conf = submodule.unit_conf.merge(self.unit_conf)
            self.ports_bind = submodule.ports_bind.merge(self.ports_bind)
            self.default_params = submodule.default_params.merge(self.default_params)

        json_dir = os.getcwd() + '/jsonconf'
        if (os.path.exists(json_dir)):
            user_input = input('Remove existing {}? y/[n]. '.format(json_dir))
            if user_input == 'y':
                shutil.rmtree(json_dir)
            else:
                raise Exception('{} exists!'.format(json_dir))
        os.mkdir(json_dir)
        self.unit_conf.to_json(json_dir + '/unit_conf.json')
        self.ports_bind.to_json(json_dir + '/port_bind.json')
        self.default_params.to_json(json_dir + '/default_params.json')

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='cmd parser')
    parser.add_argument('-p', '--pyconf', type=str, help='')
    parser.add_argument('-u', '--unitlib', type=str, help='')
    parser.add_argument('-t', '--tplconf', type=str, help='')

    args = parser.parse_args()

    if args.unitlib is not None:
        sys.path.append(args.unitlib)

    if args.pyconf is not None:
        # Generate model conf
        GenModelConf().gen_model_config(args.pyconf)
    elif args.tplconf is not None:
        # Generate template
        GenTemplate().gen_template(args.tplconf)
    else:
        GenTemplate().gen_template(_script_dir + '/intermodule.json')
    