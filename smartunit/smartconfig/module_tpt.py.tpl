import unitconf
import portsconf
import dparams

import frameconf

$extra_content

unit_conf = unitconf.UnitFactory($name, frameconf.CORE_COUNT, frameconf.CLUSTER_COUNT)
unit_bind = unitconf.UnitBind($name)
ports_conf = portsconf.PortsConf($name)
default_params = dparams.DParams($name)

# Create unit instances, you should not build units in python that bind modules
def build_units():
    # Create unit instances here.

    # unit: Unit which you want to create.
    # num: The count of units to be created.
    # location: Specify cluster and core of unit. if location is a tuple: (cluster, core); if location is an int: core. default: (0, 0).
    # unit_conf.create_unit(unit, num, location)

    # Set specified param, param_arr must equals to the count of the units you have created.
    # unit_conf.set_params(unit, param_name, param_arr, location)
    pass

# Setup how the ports bind between units.
def build_ports_conf():
    # Setup ports_conf here.

    # ports_conf.bind(outport, inport)
    pass

# Bind unit instances
def bind_units():
    # Bind unit instances here.

    # unit_bind.bind_unit(unit1, unit2, bind_matrix, location1, location2)
    pass

# Setup default params for units
def setup_dparams():
    # Setup default params for units

    # default_params.set_param(full_param_key, value)
    pass