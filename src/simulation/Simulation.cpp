#include "Simulation.hpp"
#include "sparta/ports/Port.hpp"

#include "component/Core/PerfectAlu.hpp"
#include "component/Core/PerfectLsu.hpp"
#include "component/Core/PerfectFrontend.hpp"
#include "component/Core/PerfectBackend.hpp"

#include <memory>

#include "olympia/MavisUnit.hpp"
#include "olympia/OlympiaAllocators.hpp"

namespace TimingModel {
    // Resource Factory
    sparta::ResourceFactory<TimingModel::PerfectAlu,
                            TimingModel::PerfectAlu::PerfectAluParameter> perfect_alu_factory;
    sparta::ResourceFactory<TimingModel::PerfectLsu,
                            TimingModel::PerfectLsu::PerfectLsuParameter> perfect_lsu_factory;
    sparta::ResourceFactory<TimingModel::PerfectFrontend,
                            TimingModel::PerfectFrontend::PerfectFrontendParameter> perfect_frontend_factory;
    sparta::ResourceFactory<TimingModel::PerfectBackend,
                            TimingModel::PerfectBackend::PerfectBackendParameter> perfect_backend_factory;    
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
        sparta::ResourceTreeNode* perfect_lsu_node = new sparta::ResourceTreeNode(getRoot(), 
                                    "perfect_lsu", 
                                    sparta::TreeNode::GROUP_NAME_NONE, 
                                    sparta::TreeNode::GROUP_IDX_NONE, 
                                    "perfect lsu", 
                                    &perfect_lsu_factory);

        to_delete_.emplace_back(mavis_node);
        to_delete_.emplace_back(perfect_frontend_node);
        to_delete_.emplace_back(perfect_backend_node);
        to_delete_.emplace_back(perfect_alu_node);
        to_delete_.emplace_back(perfect_lsu_node);
    }

    void Simulation::configureTree_() {
    }

    void Simulation::bindTree_() {

        sparta::bind(getRoot()->getChildAs<sparta::Port>("perfect_alu.ports.alu_backend_finish_out"), 
                     getRoot()->getChildAs<sparta::Port>("perfect_backend.ports.alu_backend_finish_in"));

        sparta::bind(getRoot()->getChildAs<sparta::Port>("perfect_alu.ports.backend_alu_inst_in"), 
                     getRoot()->getChildAs<sparta::Port>("perfect_backend.ports.backend_alu_inst_out"));

        sparta::bind(getRoot()->getChildAs<sparta::Port>("perfect_lsu.ports.lsu_backend_finish_out"), 
                     getRoot()->getChildAs<sparta::Port>("perfect_backend.ports.lsu_backend_finish_in"));

        sparta::bind(getRoot()->getChildAs<sparta::Port>("perfect_lsu.ports.backend_lsu_inst_in"), 
                     getRoot()->getChildAs<sparta::Port>("perfect_backend.ports.backend_lsu_inst_out"));

        sparta::bind(getRoot()->getChildAs<sparta::Port>("perfect_frontend.ports.fetch_backend_inst_out"), 
                     getRoot()->getChildAs<sparta::Port>("perfect_backend.ports.fetch_backend_inst_in"));
    }

    void Simulation::test() {
        getRoot()->getChildAs<sparta::ResourceTreeNode>("perfect_frontend")-> 
            getResourceAs<TimingModel::PerfectFrontend>()->Trigger();

        getRoot()->getChildAs<sparta::ResourceTreeNode>("perfect_backend")-> 
            getResourceAs<TimingModel::PerfectBackend>()->Trigger();

        auto perfect_frontend_tmp = getRoot()->getChildAs<sparta::ResourceTreeNode>("perfect_frontend")-> 
            getResourceAs<TimingModel::PerfectFrontend>();

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

        for (int i = 0; i < max_run_time; ++i) {
            getRoot()->getScheduler()->run(1);
            std::cout << "tick: " << i << std::endl << std::endl;
            if (i == 190) {
                getScheduler()->clearEvents();
                getScheduler()->stopRunning();
                std::cout << "haha" << std::endl;
            }
        }
    }
} // namespace Yihai
