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

#define InitParam(param_set, type, member) \
    member.reset(new type##Param(#member, type(), #member, param_set))

namespace TimingModel {

    extern std::map<std::string, sparta::ResourceFactoryBase*> factories_map;

    class TopoExtensions: public sparta::ExtensionsParamsOnly
    {
    public:
        static constexpr char name[] = "topo_extensions";

        using CommonTopoType = std::vector<std::vector<std::string>>;
        using CommonTopoTypeParam = sparta::Parameter<CommonTopoType>;

        using BindingTopology      = std::vector<std::string>;
        using BindingTopologyParam = sparta::Parameter<BindingTopology>;

        TopoExtensions() : sparta::ExtensionsParamsOnly() {}
        virtual ~TopoExtensions() {}

        void postCreate() override {
            sparta::ParameterSet * ps = getParameters();
            InitParam(ps, CommonTopoType, processor);
            InitParam(ps, CommonTopoType, basic_core);
            InitParam(ps, CommonTopoType, env);
            InitParam(ps, CommonTopoType, abstract_backend);
            InitParam(ps, CommonTopoType, rs_group);
            InitParam(ps, CommonTopoType, fu_group);
            InitParam(ps, CommonTopoType, abstract_lsu);
            InitParam(ps, CommonTopoType, dispatch_map);
            InitParam(ps, CommonTopoType, all_component);
            InitParam(ps, BindingTopology, binding_topology);
        }
    private:
        std::unique_ptr<CommonTopoTypeParam> processor;
        std::unique_ptr<CommonTopoTypeParam> basic_core;
        std::unique_ptr<CommonTopoTypeParam> env;
        std::unique_ptr<CommonTopoTypeParam> abstract_backend;
        std::unique_ptr<CommonTopoTypeParam> rs_group;
        std::unique_ptr<CommonTopoTypeParam> fu_group;
        std::unique_ptr<CommonTopoTypeParam> abstract_lsu;
        std::unique_ptr<CommonTopoTypeParam> dispatch_map;
        std::unique_ptr<CommonTopoTypeParam> all_component;
        std::unique_ptr<BindingTopologyParam> binding_topology;
    };

    TopoExtensions::CommonTopoType getCommonTopology(sparta::TreeNode * node, std::string topo_name);
    TopoExtensions::BindingTopology getBindingTopology(sparta::TreeNode * node);
}

#undef InitParam