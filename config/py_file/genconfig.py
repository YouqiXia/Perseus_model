import unitlib
import json

import unit_config

if __name__ == "__main__":
    units = unitlib.UnitsSet()
    config = unit_config.Config(units)
    config.gen_topo_json()
    config.gen_units_map_json()
    config.gen_test()











            
        