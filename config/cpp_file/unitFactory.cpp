//
// Created by yzhang on 9/1/24.
//

#include "unitFactory.hpp"
#include "basic/Instruction.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

PYBIND11_MODULE(unitlib, m) {
    PybindInterface::UnitsSet unit_set;

    pybind11::class_<sparta::ParameterBase>(m, "ParameterBase")
            .def("getBoolResource", [](const sparta::ParameterBase* self) {
                return self->getValueAs<bool>(); })
            .def("getInt64Resource", [](const sparta::ParameterBase* self) {
                return self->getValueAs<uint64_t>(); })
            .def("getInt32Resource", [](const sparta::ParameterBase* self) {
                return self->getValueAs<uint32_t>(); })
            .def("getStringResource", [](const sparta::ParameterBase* self) {
                return self->getValueAs<std::string>(); })
            .def("getVectorStringResource", [](const sparta::ParameterBase* self) {
                return self->getValueAs<std::vector<std::string>>(); })
            .def("getName", [](const sparta::ParameterBase* self) {
                return self->getName(); })
            .def("getTypeName", [](const sparta::ParameterBase* self) {
                return self->getTypeName(); })
            ;

    pybind11::class_<PybindInterface::UnitsSet::UnitStatistics>(m, "UnitStatistics")
            .def(pybind11::init<>())
            .def_readonly("ports_info", &PybindInterface::UnitsSet::UnitStatistics::ports_info)
            .def_readonly("params_info", &PybindInterface::UnitsSet::UnitStatistics::params_info)
            ;

    pybind11::class_<PybindInterface::UnitsSet>(m, "UnitsSet")
            .def(pybind11::init<>())
            .def("GetUnitsInfo", &PybindInterface::UnitsSet::GetUnitsInfo)
            .def("GetNameSet", &PybindInterface::UnitsSet::GetNameSet)
            .def("GetPortSet", &PybindInterface::UnitsSet::GetPortSet)
            ;
// units
    for (const std::string name : unit_set.GetNameSet()) {
        const char * attr_name = name.c_str();
        m.def_submodule("units").attr(attr_name) = attr_name;
    }

// func types
    for (int i = static_cast<int>(TimingModel::FuncType::NO_TYPE); i < static_cast<int>(TimingModel::FuncType::LAST); ++i) {
        TimingModel::FuncType func_type = static_cast<TimingModel::FuncType>(i);
        if (func_type == TimingModel::FuncType::NO_TYPE) {
            continue;
        }
        std::string attr_name_string = TimingModel::funcTypeToString(func_type);
        const char* attr_name = attr_name_string.c_str();
        m.def_submodule("func_type").attr(attr_name) = attr_name;
    }

// ports
    for (const auto& unit_info_pair: unit_set.GetUnitsInfo()) {
        const char* unit_name = unit_info_pair.first.c_str();
        for (const auto& ports_info_pair: unit_info_pair.second.ports_info) {
            const char* port_direction = ports_info_pair.first.c_str();
                for(const auto& port_name: ports_info_pair.second) {
                    const char* attr_name = port_name.c_str();
                    m.def_submodule("ports").def_submodule(unit_name).def_submodule(port_direction)
                        .attr(attr_name) = port_name;
                }
        }
    }

// params
    for (const auto& unit_info_pair: unit_set.GetUnitsInfo()) {
        const char* unit_name = unit_info_pair.first.c_str();
        for (const auto& params_info_pair: unit_info_pair.second.params_info) {
            const char* param_name = params_info_pair.first.c_str();
            m.def_submodule("params").def_submodule(unit_name).attr(param_name) = param_name;
        }
    }
}

