import unitconf
import portsbind
import dparams

import frameconf

$extra_content

unit_conf = unitconf.UnitFactory($name, frameconf.CORE_COUNT, frameconf.CLUSTER_COUNT)
ports_bind = portsbind.PortsBind($name)
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

# Bind ports of the unit instances
def bind_ports():
    # Bind unit ports here.

    # ports_bind.bind(port1, port2, bind_matrix, location1, location2)
    pass

# Setup default params for units
def setup_dparams():
    # Setup default params for units

    # default_params.set_param(full_param_key, value)
    pass