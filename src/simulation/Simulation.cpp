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

#include "olympia/MavisUnit.hpp"
#include "olympia/OlympiaAllocators.hpp"

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

        sparta::ResourceTreeNode* mavis_node = new sparta::ResourceTreeNode(getRoot(),
                                                                            MavisUnit::name,
                                                                            sparta::TreeNode::GROUP_NAME_NONE, 
                                                                            sparta::TreeNode::GROUP_IDX_NONE, 
                                                                            "mavis unit", 
                                                                            &mavis_fact);
                                                                            
        sparta::ResourceTreeNode* perfect_frontend_node = new sparta::ResourceTreeNode(getRoot(), 
                                    "perfect_frontend", 
                                    sparta::TreeNode::GROUP_NAME_NONE, 
                                    sparta::TreeNode::GROUP_IDX_NONE, 
                                    "perfect frontend", 
                                    &perfect_frontend_factory);
        perfect_frontend_node->getParameterSet()->getParameter("input_file")->setValueFromString(workload_);

        sparta::ResourceTreeNode* perfect_backend_node = new sparta::ResourceTreeNode(getRoot(), 
                                    "perfect_backend", 
                                    sparta::TreeNode::GROUP_NAME_NONE, 
                                    sparta::TreeNode::GROUP_IDX_NONE, 
                                    "perfect backend", 
                                    &perfect_backend_factory);
        sparta::ResourceTreeNode* perfect_alu_node = new sparta::ResourceTreeNode(getRoot(), 
                                    PerfectAlu::name, 
                                    sparta::TreeNode::GROUP_NAME_NONE, 
                                    sparta::TreeNode::GROUP_IDX_NONE, 
                                    "perfect alu", 
                                    &perfect_alu_factory);
        // sparta::ResourceTreeNode* perfect_lsu_node = new sparta::ResourceTreeNode(getRoot(), 
        //                             "perfect_lsu", 
        //                             sparta::TreeNode::GROUP_NAME_NONE, 
        //                             sparta::TreeNode::GROUP_IDX_NONE, 
        //                             "perfect lsu", 
        //                             &perfect_lsu_factory);
        sparta::ResourceTreeNode* abstract_lsu_node = new sparta::ResourceTreeNode(getRoot(), 
                                    "abstract_lsu", 
                                    sparta::TreeNode::GROUP_NAME_NONE, 
                                    sparta::TreeNode::GROUP_IDX_NONE, 
                                    "abstract_lsu", 
                                    &abstract_lsu_factory);
        sparta::ResourceTreeNode* l1dcache_node = new sparta::ResourceTreeNode(getRoot(), 
                                    "l1d_cache", 
                                    sparta::TreeNode::GROUP_NAME_NONE, 
                                    sparta::TreeNode::GROUP_IDX_NONE, 
                                    "l1d_cache", 
                                    &base_cache_factory);
        sparta::ResourceTreeNode* l2cache_node = new sparta::ResourceTreeNode(getRoot(), 
                                    "l2_cache", 
                                    sparta::TreeNode::GROUP_NAME_NONE, 
                                    sparta::TreeNode::GROUP_IDX_NONE, 
                                    "l2_cache", 
                                    &base_cache_factory);
        sparta::ResourceTreeNode* mem_node = new sparta::ResourceTreeNode(getRoot(), 
                                    "abstract_mem", 
                                    sparta::TreeNode::GROUP_NAME_NONE, 
                                    sparta::TreeNode::GROUP_IDX_NONE, 
                                    "abstract_mem", 
                                    &abstract_memroy_factory);                          

        to_delete_.emplace_back(l1dcache_node);
        to_delete_.emplace_back(l2cache_node);
        to_delete_.emplace_back(mem_node); 
        to_delete_.emplace_back(mavis_node);
        to_delete_.emplace_back(perfect_frontend_node);
        to_delete_.emplace_back(perfect_backend_node);
        to_delete_.emplace_back(perfect_alu_node);
        // to_delete_.emplace_back(perfect_lsu_node);
        to_delete_.emplace_back(abstract_lsu_node);
    }

    void Simulation::configureTree_() {
    }

    void Simulation::bindTree_() {

        sparta::bind(getRoot()->getChildAs<sparta::Port>("perfect_alu.ports.alu_backend_finish_out"), 
                     getRoot()->getChildAs<sparta::Port>("perfect_backend.ports.alu_backend_finish_in"));

        sparta::bind(getRoot()->getChildAs<sparta::Port>("perfect_alu.ports.backend_alu_inst_in"), 
                     getRoot()->getChildAs<sparta::Port>("perfect_backend.ports.backend_alu_inst_out"));

        sparta::bind(getRoot()->getChildAs<sparta::Port>("abstract_lsu.ports.lsu_backend_finish_out"), 
                     getRoot()->getChildAs<sparta::Port>("perfect_backend.ports.lsu_backend_finish_in"));

        sparta::bind(getRoot()->getChildAs<sparta::Port>("abstract_lsu.ports.backend_lsu_inst_in"), 
                     getRoot()->getChildAs<sparta::Port>("perfect_backend.ports.backend_lsu_inst_out"));

        sparta::bind(getRoot()->getChildAs<sparta::Port>("abstract_lsu.ports.lsu_backend_wr_data_out"), 
                     getRoot()->getChildAs<sparta::Port>("perfect_backend.ports.lsu_backend_wr_data_in"));
        
        sparta::bind(getRoot()->getChildAs<sparta::Port>("abstract_lsu.ports.backend_lsu_rob_idx_wakeup_in"), 
                     getRoot()->getChildAs<sparta::Port>("perfect_backend.ports.backend_lsu_rob_idx_wakeup_out"));

        sparta::bind(getRoot()->getChildAs<sparta::Port>("perfect_frontend.ports.fetch_backend_inst_out"), 
                     getRoot()->getChildAs<sparta::Port>("perfect_backend.ports.fetch_backend_inst_in"));

        // Credits
        sparta::bind(getRoot()->getChildAs<sparta::Port>("perfect_frontend.ports.fetch_backend_credit_in"), 
                     getRoot()->getChildAs<sparta::Port>("perfect_backend.ports.fetch_backend_credit_out"));

        sparta::bind(getRoot()->getChildAs<sparta::Port>("perfect_backend.ports.lsu_backend_credit_in"), 
                     getRoot()->getChildAs<sparta::Port>("abstract_lsu.ports.backend_lsu_credit_out"));

        sparta::bind(getRoot()->getChildAs<sparta::Port>("perfect_backend.ports.alu_backend_credit_in"), 
                     getRoot()->getChildAs<sparta::Port>("perfect_alu.ports.backend_alu_credit_out"));

        // precedes
        // sparta::bind(getRoot()->getChildAs<sparta::Port>("perfect_lsu.ports.in_downstream_credit"), 
        //              getRoot()->getChildAs<sparta::Port>("l1d_cache.ports.out_upstream_credit"));
        // sparta::bind(getRoot()->getChildAs<sparta::Port>("perfect_lsu.ports.in_access_resp"), 
        //              getRoot()->getChildAs<sparta::Port>("l1d_cache.ports.out_access_resp"));
        // sparta::bind(getRoot()->getChildAs<sparta::Port>("perfect_lsu.ports.out_access_req"), 
        //              getRoot()->getChildAs<sparta::Port>("l1d_cache.ports.in_access_req"));

        // sparta::bind(getRoot()->getChildAs<sparta::Port>("l1d_cache.ports.in_downstream_credit"), 
        //              getRoot()->getChildAs<sparta::Port>("abstract_mem.ports.out_upstream_credit"));
        // sparta::bind(getRoot()->getChildAs<sparta::Port>("l1d_cache.ports.in_access_resp"), 
        //              getRoot()->getChildAs<sparta::Port>("abstract_mem.ports.mem_resp_out"));
        // sparta::bind(getRoot()->getChildAs<sparta::Port>("l1d_cache.ports.out_access_req"), 
        //              getRoot()->getChildAs<sparta::Port>("abstract_mem.ports.mem_req_in"));

        // perfect lsu to l1d cache //
        // sparta::bind(getRoot()->getChildAs<sparta::Port>("perfect_lsu.ports.in_downstream_credit"), 
        //              getRoot()->getChildAs<sparta::Port>("l1d_cache.ports.out_upstream_credit"));
        // sparta::bind(getRoot()->getChildAs<sparta::Port>("perfect_lsu.ports.in_access_resp"), 
        //              getRoot()->getChildAs<sparta::Port>("l1d_cache.ports.out_access_resp"));
        // sparta::bind(getRoot()->getChildAs<sparta::Port>("perfect_lsu.ports.out_access_req"), 
        //              getRoot()->getChildAs<sparta::Port>("l1d_cache.ports.in_access_req"));
        // perfect lsu to l1d cache //

        // abstract lsu to l1d cache //
        sparta::bind(getRoot()->getChildAs<sparta::Port>("abstract_lsu.ports.l1d_cache_lsu_credit_in"), 
                     getRoot()->getChildAs<sparta::Port>("l1d_cache.ports.out_upstream_credit"));
        sparta::bind(getRoot()->getChildAs<sparta::Port>("abstract_lsu.ports.l1d_cache_lsu_in"), 
                     getRoot()->getChildAs<sparta::Port>("l1d_cache.ports.out_access_resp"));
        sparta::bind(getRoot()->getChildAs<sparta::Port>("abstract_lsu.ports.lsu_l1d_cache_out"), 
                     getRoot()->getChildAs<sparta::Port>("l1d_cache.ports.in_access_req"));
        // abstract lsu to l1d cache //

        sparta::bind(getRoot()->getChildAs<sparta::Port>("l1d_cache.ports.in_downstream_credit"), 
                     getRoot()->getChildAs<sparta::Port>("l2_cache.ports.out_upstream_credit"));
        sparta::bind(getRoot()->getChildAs<sparta::Port>("l1d_cache.ports.in_access_resp"), 
                     getRoot()->getChildAs<sparta::Port>("l2_cache.ports.out_access_resp"));
        sparta::bind(getRoot()->getChildAs<sparta::Port>("l1d_cache.ports.out_access_req"), 
                     getRoot()->getChildAs<sparta::Port>("l2_cache.ports.in_access_req"));

        sparta::bind(getRoot()->getChildAs<sparta::Port>("l2_cache.ports.in_downstream_credit"), 
                     getRoot()->getChildAs<sparta::Port>("abstract_mem.ports.out_upstream_credit"));
        sparta::bind(getRoot()->getChildAs<sparta::Port>("l2_cache.ports.in_access_resp"), 
                     getRoot()->getChildAs<sparta::Port>("abstract_mem.ports.mem_resp_out"));
        sparta::bind(getRoot()->getChildAs<sparta::Port>("l2_cache.ports.out_access_req"), 
                     getRoot()->getChildAs<sparta::Port>("abstract_mem.ports.mem_req_in"));

    }

    void Simulation::test() {

        // for (int i = 0; i < 128; ++i) {
        //     InstPtr inst {new InstInfo};
        //     if (!(i%5)) {
        //         inst->setFuType(FuncType::LDU);
        //     } else if (!(i%7)) {
        //         inst->setFuType(FuncType::STU);
        //     } else {
        //         inst->setFuType(FuncType::ALU);
        //     }
        //     inst->setPC(i);
        //     perfect_frontend_tmp->SetInst(inst);
        // }

        // for (int i = 0; i < max_run_time; ++i) {
        //     getRoot()->getScheduler()->run(1);
        //     std::cout << "tick: " << i << std::endl << std::endl;
        // }
    }
} // namespace Yihai
