#pragma once
#include "Core/Frontend/PerfectFrontend.hpp"

#include "Core/Backend/RenamingStage.hpp"
#include "Core/Backend/Rob.hpp"
#include "Core/Backend/DispatchStage.hpp"
#include "Core/Backend/ReservationStation.hpp"

#include "Core/FuncUnit/PerfectAlu.hpp"
#include "Core/FuncUnit/PerfectLsu.hpp"
#include "Core/FuncUnit/WriteBackStage.hpp"
#include "sparta/simulation/TreeNodeExtensions.hpp"

namespace TimingModel {

    extern std::map<std::string, sparta::ResourceFactoryBase*> factories_map;

    class TopoExtensions: public sparta::ExtensionsParamsOnly
    {
    public:
        static constexpr char name[] = "topo_extensions";

        using ModuleTopology      = std::vector<std::string>;
        using ModuleTopologyParam = sparta::Parameter<ModuleTopology>;
        using BindingTopology      = std::vector<std::string>;
        using BindingTopologyParam = sparta::Parameter<BindingTopology>;

        using FuMap      = std::vector<std::vector<std::string>>;
        using FuMapParam = sparta::Parameter<FuMap>;
        using FuCreditMap      = std::vector<std::vector<std::string>>;
        using FuCreditMapParam = sparta::Parameter<FuCreditMap>;

        TopoExtensions() : sparta::ExtensionsParamsOnly() {}
        virtual ~TopoExtensions() {}

        void postCreate() override {
            sparta::ParameterSet * ps = getParameters();

            module_topology_.
                reset(new ModuleTopologyParam("module_topology", ModuleTopology(),
                                                 "Module Topology", ps));
            binding_topology_.
                reset(new BindingTopologyParam("binding_topology", BindingTopology(),
                                                 "Binding Topology", ps));
            fu_map_.
                reset(new FuMapParam("fu_map", FuMap(), "function unit map", ps));
            
            fu_credit_map_.
                reset(new FuCreditMapParam("fu_credit_map", FuCreditMap(), "function unit credit map", ps));
        }
    private:
        std::unique_ptr<ModuleTopologyParam> module_topology_;
        std::unique_ptr<BindingTopologyParam> binding_topology_;
        std::unique_ptr<FuMapParam> fu_map_;
        std::unique_ptr<FuCreditMapParam> fu_credit_map_;
    };


    TopoExtensions::ModuleTopology getModuleTopology(sparta::TreeNode * node);
    TopoExtensions::BindingTopology getBindingTopology(sparta::TreeNode * node);
    TopoExtensions::FuMap getFuMap(sparta::TreeNode * node);
    TopoExtensions::FuCreditMap getFuCreditMap(sparta::TreeNode * node);
}