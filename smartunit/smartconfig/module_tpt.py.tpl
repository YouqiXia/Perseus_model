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
    # Create unit instances here
    pass

# Setup how the ports bind between units.
def build_ports_conf():
    # Setup ports_conf here
    pass

# Bind unit instances
def bind_units():
    # Bind unit instances here
    pass

# Setup default params for units
def setup_dparams():
    # Setup default params for units
    pass