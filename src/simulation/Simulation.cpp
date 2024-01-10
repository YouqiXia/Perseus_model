#include "Simulation.hpp"
#include "sparta/ports/Port.hpp"

#include "Core/Backend/PerfectAlu.hpp"
#include "Core/Backend/PerfectLsu.hpp"
#include "Core/Backend/AbstractLsu.hpp"
#include "Core/Frontend/PerfectFrontend.hpp"
#include "Core/Backend/PerfectBackend.hpp"
#include "uncore/cache/BaseCache.hpp"
#include "uncore/memory/AbstractMemory.hpp"
#include <memory>
#include <cstdint>

#include "olympia/MavisUnit.hpp"
#include "olympia/OlympiaAllocators.hpp"
#include "FuncUnits.hpp"

namespace TimingModel {
    // Resource Factory
    sparta::ResourceFactory<TimingModel::PerfectAlu,
                            TimingModel::PerfectAlu::PerfectAluParameter> perfect_alu_factory;
    // sparta::ResourceFactory<TimingModel::PerfectLsu,
    //                         TimingModel::PerfectLsu::PerfectLsuParameter> perfect_lsu_factory;
    sparta::ResourceFactory<TimingModel::AbstractLsu,
                            TimingModel::AbstractLsu::AbstractLsuParameter> abstract_lsu_factory;
    sparta::ResourceFactory<TimingModel::PerfectFrontend,
                            TimingModel::PerfectFrontend::PerfectFrontendParameter> perfect_frontend_factory;
    sparta::ResourceFactory<TimingModel::PerfectBackend,
                            TimingModel::PerfectBackend::PerfectBackendParameter> perfect_backend_factory;
    sparta::ResourceFactory<TimingModel::BaseCache,
                            TimingModel::BaseCache::BaseCacheParameterSet> base_cache_factory;
    sparta::ResourceFactory<TimingModel::AbstractMemroy,
                            TimingModel::AbstractMemroy::AbstractMemroyParameterSet> abstract_memroy_factory;    
    MavisFactoy mavis_fact;
    std::unique_ptr<OlympiaAllocators> global_allocators;

    std::map<std::string, sparta::ResourceFactoryBase*> factories_map = {
        {"mavis",                           &mavis_fact},
        {PerfectFrontend::name,             &perfect_frontend_factory},
        {PerfectBackend::name,              &perfect_backend_factory},
        {PerfectAlu::name,                  &perfect_alu_factory},
        {AbstractLsu::name,                 &abstract_lsu_factory},
        {"l1d_cache",                       &base_cache_factory},
        {"l2_cache",                        &base_cache_factory},
        {"abstract_memory",                 &abstract_memroy_factory},
    };

    // Simulation
    Simulation::Simulation(sparta::Scheduler& Sched,
                           const std::string workload) :
        sparta::app::Simulation("Yihai_test", &Sched),
        workload_(workload)
    {
        sparta::StartupEvent(getRoot(), CREATE_SPARTA_HANDLER(Simulation, test));
    }

    Simulation::~Simulation() {
        getRoot()->enterTeardown();
    }

    void Simulation::buildTree_() {

        global_allocators.reset(new OlympiaAllocators(getRoot()));
        getRoot()->addExtensionFactory(TimingModel::TopoExtensions::name,
                                        [&]()->sparta::TreeNode::ExtensionsBase * {return new TimingModel::TopoExtensions();});
        auto module_topo = TimingModel::getModuleTopology(getRoot());
        buildModuleTopology_(module_topo, getRoot());
        setFuCfgFromExtensions(getRoot());
        BuildFuncRelatives_();
    }

    void Simulation::BuildFuncRelatives_() {
        // rs
        // FuncMap& fu_map = getFuncMap();
        // for (auto& func_pair: fu_map) {
        //     auto * reservation_station_tmp = new sparta::ResourceTreeNode(getRoot(),
        //                                                                   "rs_"+func_pair.first,
        //                                                                   sparta::TreeNode::GROUP_NAME_NONE,
        //                                                                   sparta::TreeNode::GROUP_IDX_NONE,
        //                                                                   "reservation station",
        //                                                                   &reservation_station_factory);
        //     reservation_station_tmp->getParameterSet()->getParameter("rs_func_type")->setValueFromString(func_pair.first);

        //     sparta::ResourceTreeNode * func_units_tmp;
        //     if (func_pair.first == "LSU") {
        //         func_units_tmp = new sparta::ResourceTreeNode(getRoot(),
        //                                                              "func_"+func_pair.first,
        //                                                              sparta::TreeNode::GROUP_NAME_NONE,
        //                                                              sparta::TreeNode::GROUP_IDX_NONE,
        //                                                              func_pair.first,
        //                                                              &perfect_lsu_factory);
        //     } else {
        //         func_units_tmp = new sparta::ResourceTreeNode(getRoot(),
        //                                                              "func_"+func_pair.first,
        //                                                              sparta::TreeNode::GROUP_NAME_NONE,
        //                                                              sparta::TreeNode::GROUP_IDX_NONE,
        //                                                              func_pair.first,
        //                                                              &perfect_alu_factory);
        //     }
        //     func_units_tmp->getParameterSet()->getParameter("func_type")->setValueFromString(func_pair.first);
        //     to_delete_.emplace_back(reservation_station_tmp);
        //     to_delete_.emplace_back(func_units_tmp);
        // }
    }

    void Simulation::configureTree_() {
    }

    void Simulation::bindTree_() {
        auto binding_topo = TimingModel::getBindingTopology(getRoot());
        bindBindingTopology_(binding_topo);

        // FuncMap& fu_map = getFuncMap();
        // // dispatch stage -> separate rs
        // for (auto& func_pair: fu_map) {
        //     sparta::bind(getRoot()->getChildAs<sparta::Port>("dispatch_stage.ports.dispatch_"+func_pair.first+"_rs_out"),
        //                  getRoot()->getChildAs<sparta::Port>("rs_"+func_pair.first+".ports.preceding_reservation_inst_in"));
        //     // separate rs -> function units
        //     sparta::bind(getRoot()->getChildAs<sparta::Port>("rs_"+func_pair.first+".ports.reservation_following_inst_out"),
        //                  getRoot()->getChildAs<sparta::Port>("func_"+func_pair.first+".ports.preceding_func_inst_in"));

        //     // function units -> write back arbiter
        //     sparta::bind(getRoot()->getChildAs<sparta::Port>("func_"+func_pair.first+".ports.func_following_finish_out"),
        //                  getRoot()->getChildAs<sparta::Port>("write_back_stage.ports."+func_pair.first+"_write_back_port_in"));
        //     // write back -> separate rs
        //     sparta::bind(getRoot()->getChildAs<sparta::Port>("write_back_stage.ports.write_back_following_port_out"),
        //                  getRoot()->getChildAs<sparta::Port>("rs_"+func_pair.first+".ports.forwarding_reservation_inst_in"));

        //     // separate rs -> dispatch stage
        //     sparta::bind(getRoot()->getChildAs<sparta::Port>("rs_"+func_pair.first+".ports.reservation_preceding_credit_out"),
        //                  getRoot()->getChildAs<sparta::Port>("dispatch_stage.ports."+func_pair.first+"_rs_dispatch_credit_in"));
        //     // write back -> separate rs
        //     sparta::bind(getRoot()->getChildAs<sparta::Port>("write_back_stage.ports."+ func_pair.first + "_rs_credit_out"),
        //                  getRoot()->getChildAs<sparta::Port>("rs_"+func_pair.first+".ports.following_reservation_credit_in"));
        // }
    }

    void Simulation::test() {}

    void Simulation::buildModuleTopology_(TopoExtensions::ModuleTopology& topo, sparta::TreeNode* parent){
        for(auto node_name : topo ){
            auto node_tmp = new sparta::ResourceTreeNode(parent,
                                                          node_name,
                                                          sparta::TreeNode::GROUP_NAME_NONE,
                                                          sparta::TreeNode::GROUP_IDX_NONE,
                                                          node_name,

                                                          factories_map[node_name]);
            to_delete_.emplace_back(node_tmp);
            if(node_name.compare("perfect_frontend") == 0){
                node_tmp->getParameterSet()->getParameter("input_file")->setValueFromString(workload_);
            }
        }
    }

    void Simulation::bindBindingTopology_(TopoExtensions::BindingTopology& topo){
        for(auto i=0u; i<topo.size(); ){
            sparta::bind(getRoot()->getChildAs<sparta::Port>(topo[i]),
                        getRoot()->getChildAs<sparta::Port>(topo[i+1]));
            i += 2;
        }
    }

} // namespace Yihai
