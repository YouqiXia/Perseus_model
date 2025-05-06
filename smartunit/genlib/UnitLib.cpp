#include "UnitLib.hpp"

#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smartunit/registered/UnitRegister.hpp"

PYBIND11_MODULE(UnitLib, m) {
    TimingModel::UnitRegister::doAllRegister();
    PybindInterface::UnitsSet unit_set;
    
    for (const auto &unit_node : unit_set.getUnitNodes()) {
        std::string type_str = registertype_to_str(unit_node.m_type);
        const char *unit_name = unit_node.m_name.c_str();
        
        /* module */
        m.def_submodule(type_str.c_str()).attr("name") = type_str.c_str();

        /* unit */
        m.def_submodule(type_str.c_str()).def_submodule(unit_name);
        m.def_submodule(type_str.c_str()).def_submodule(unit_name).attr("name") = type_str + "." + unit_node.m_name;

        /* inports */
        for (const auto &port_str : unit_node.m_inports) {
            m.def_submodule(type_str.c_str()).def_submodule(unit_name)
             .def_submodule("inports").attr(port_str.c_str()) = (type_str + "." + unit_node.m_name + "|" + port_str).c_str();
        }

        /* outports */
        for (const auto &port_str : unit_node.m_outports) {
            m.def_submodule(type_str.c_str()).def_submodule(unit_name)
             .def_submodule("outports").attr(port_str.c_str()) = (type_str + "." + unit_node.m_name + "|" + port_str).c_str();
        }

        /* params */
        for (const auto &str : unit_node.m_params) {
            m.def_submodule(type_str.c_str()).def_submodule(unit_name)
             .def_submodule("params").attr(str.c_str()) = (type_str + "." + unit_node.m_name + "|" + str).c_str();
        }
    }
}