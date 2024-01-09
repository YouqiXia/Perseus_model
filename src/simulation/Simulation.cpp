#include "Simulation.hpp"
#include "sparta/ports/Port.hpp"

#include <memory>

#include "olympia/MavisUnit.hpp"
#include "olympia/OlympiaAllocators.hpp"

#include "FuncUnits.hpp"

#include "Core/Frontend/PerfectFrontend.hpp"

#include "Core/Backend/RenamingStage.hpp"
#include "Core/Backend/Rob.hpp"
#include "Core/Backend/DispatchStage.hpp"
#include "Core/Backend/ReservationStation.hpp"

#include "Core/FuncUnit/PerfectAlu.hpp"
#include "Core/FuncUnit/PerfectLsu.hpp"
#include "Core/FuncUnit/WriteBackStage.hpp"

namespace TimingModel {
    // Resource Factory
    // Frontend
    sparta::ResourceFactory<TimingModel::PerfectFrontend,
            TimingModel::PerfectFrontend::PerfectFrontendParameter> perfect_frontend_factory;

    // Backend
    sparta::ResourceFactory<TimingModel::RenamingStage,
            TimingModel::RenamingStage::RenamingParameter> renaming_stage_factory;

    sparta::ResourceFactory<TimingModel::Rob,
            TimingModel::Rob::RobParameter> rob_factory;

    sparta::ResourceFactory<TimingModel::DispatchStage,
            TimingModel::DispatchStage::DispatchStageParameter> dispatch_stage_factory;

    sparta::ResourceFactory<TimingModel::ReservationStation,
            TimingModel::ReservationStation::ReservationStationParameter> reservation_station_factory;

    // Function Unit
    sparta::ResourceFactory<TimingModel::PerfectAlu,
                            TimingModel::PerfectAlu::PerfectAluParameter> perfect_alu_factory;

    sparta::ResourceFactory<TimingModel::PerfectLsu,
                            TimingModel::PerfectLsu::PerfectLsuParameter> perfect_lsu_factory;

    sparta::ResourceFactory<TimingModel::WriteBackStage,
            TimingModel::WriteBackStage::WriteBackStageParameter> write_back_stage_factory;

    // third party units
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

        auto* mavis_node = new sparta::ResourceTreeNode(getRoot(),
                                                                            MavisUnit::name,
                                                                            sparta::TreeNode::GROUP_NAME_NONE, 
                                                                            sparta::TreeNode::GROUP_IDX_NONE, 
                                                                            "mavis unit", 
                                                                            &mavis_fact);
                                                                            
        auto* perfect_frontend_node = new sparta::ResourceTreeNode(getRoot(),
                                    PerfectFrontend::name,
                                    sparta::TreeNode::GROUP_NAME_NONE, 
                                    sparta::TreeNode::GROUP_IDX_NONE, 
                                    "perfect frontend", 
                                    &perfect_frontend_factory);
        perfect_frontend_node->getParameterSet()->getParameter("input_file")->setValueFromString(workload_);

        auto* renaming_stage_node = new sparta::ResourceTreeNode(getRoot(),
                                                                 RenamingStage::name,
                                                                 sparta::TreeNode::GROUP_NAME_NONE,
                                                                 sparta::TreeNode::GROUP_IDX_NONE,
                                                                 "renaming stage",
                                                                 &renaming_stage_factory);

        auto* rob_node = new sparta::ResourceTreeNode(getRoot(),
                                                   Rob::name,
                                                   sparta::TreeNode::GROUP_NAME_NONE,
                                                   sparta::TreeNode::GROUP_IDX_NONE,
                                                   "rob",
                                                   &rob_factory);

        auto* dispatch_stage_node = new sparta::ResourceTreeNode(getRoot(),
                                                              DispatchStage::name,
                                                              sparta::TreeNode::GROUP_NAME_NONE,
                                                              sparta::TreeNode::GROUP_IDX_NONE,
                                                              "dispatch stage",
                                                              &dispatch_stage_factory);

        BuildFuncRelatives_();

        auto* write_back_node = new sparta::ResourceTreeNode(getRoot(),
                                                              WriteBackStage::name,
                                                              sparta::TreeNode::GROUP_NAME_NONE,
                                                              sparta::TreeNode::GROUP_IDX_NONE,
                                                              "write back stage",
                                                              &write_back_stage_factory);

        to_delete_.emplace_back(mavis_node);
        to_delete_.emplace_back(perfect_frontend_node);
        to_delete_.emplace_back(renaming_stage_node);
        to_delete_.emplace_back(rob_node);
        to_delete_.emplace_back(dispatch_stage_node);
        to_delete_.emplace_back(write_back_node);
    }

    void Simulation::BuildFuncRelatives_() {
        // rs
        for (auto& func_pair: getFuncMap()) {
            auto * reservation_station_tmp = new sparta::ResourceTreeNode(getRoot(),
                                                                          "rs_"+func_pair.first,
                                                                          sparta::TreeNode::GROUP_NAME_NONE,
                                                                          sparta::TreeNode::GROUP_IDX_NONE,
                                                                          "reservation station",
                                                                          &reservation_station_factory);
            reservation_station_tmp->getParameterSet()->getParameter("rs_func_type")->setValueFromString(func_pair.first);

            sparta::ResourceTreeNode * func_units_tmp;
            if (func_pair.first == "LSU") {
                func_units_tmp = new sparta::ResourceTreeNode(getRoot(),
                                                                     "func_"+func_pair.first,
                                                                     sparta::TreeNode::GROUP_NAME_NONE,
                                                                     sparta::TreeNode::GROUP_IDX_NONE,
                                                                     func_pair.first,
                                                                     &perfect_lsu_factory);
            } else {
                func_units_tmp = new sparta::ResourceTreeNode(getRoot(),
                                                                     "func_"+func_pair.first,
                                                                     sparta::TreeNode::GROUP_NAME_NONE,
                                                                     sparta::TreeNode::GROUP_IDX_NONE,
                                                                     func_pair.first,
                                                                     &perfect_alu_factory);
            }
            func_units_tmp->getParameterSet()->getParameter("func_type")->setValueFromString(func_pair.first);
            to_delete_.emplace_back(reservation_station_tmp);
            to_delete_.emplace_back(func_units_tmp);
        }
    }

    void Simulation::configureTree_() {
    }

    void Simulation::bindTree_() {

        /* Instruction */
        // frontend -> renaming stage
        sparta::bind(getRoot()->getChildAs<sparta::Port>("perfect_frontend.ports.fetch_backend_inst_out"),
                getRoot()->getChildAs<sparta::Port>("renaming_stage.ports.preceding_renaming_inst_in"));

        // renaming stage -> rob
        sparta::bind(getRoot()->getChildAs<sparta::Port>("renaming_stage.ports.renaming_following_inst_out"),
                     getRoot()->getChildAs<sparta::Port>("rob.ports.preceding_rob_inst_in"));

        // renaming stage -> dispatch stage
        sparta::bind(getRoot()->getChildAs<sparta::Port>("renaming_stage.ports.renaming_following_inst_out"),
                     getRoot()->getChildAs<sparta::Port>("dispatch_stage.ports.preceding_dispatch_inst_in"));

        // dispatch stage -> separate rs
        for (auto& func_pair: getFuncMap()) {
            sparta::bind(getRoot()->getChildAs<sparta::Port>("dispatch_stage.ports.dispatch_"+func_pair.first+"_rs_out"),
                         getRoot()->getChildAs<sparta::Port>("rs_"+func_pair.first+".ports.preceding_reservation_inst_in"));
        }

        // separate rs -> function units
        for (auto& func_pair: getFuncMap()) {
            sparta::bind(getRoot()->getChildAs<sparta::Port>("rs_"+func_pair.first+".ports.reservation_following_inst_out"),
                         getRoot()->getChildAs<sparta::Port>("func_"+func_pair.first+".ports.preceding_func_inst_in"));
        }

        // function units -> write back arbiter
        for (auto& func_pair: getFuncMap()) {
            sparta::bind(getRoot()->getChildAs<sparta::Port>("func_"+func_pair.first+".ports.func_following_finish_out"),
                         getRoot()->getChildAs<sparta::Port>("write_back_stage.ports."+func_pair.first+"_write_back_port_in"));
        }

        // write back -> rob
        sparta::bind(getRoot()->getChildAs<sparta::Port>("write_back_stage.ports.write_back_following_port_out"),
                     getRoot()->getChildAs<sparta::Port>("rob.ports.write_back_rob_finish_in"));

        // write back -> dispatch stage
        sparta::bind(getRoot()->getChildAs<sparta::Port>("write_back_stage.ports.write_back_following_port_out"),
                     getRoot()->getChildAs<sparta::Port>("dispatch_stage.ports.write_back_dispatch_port_in"));

        // write back -> separate rs
        for (auto& func_pair: getFuncMap()) {
            sparta::bind(getRoot()->getChildAs<sparta::Port>("write_back_stage.ports.write_back_following_port_out"),
                         getRoot()->getChildAs<sparta::Port>("rs_"+func_pair.first+".ports.forwarding_reservation_inst_in"));
        }

        // rob -> renaming stage
        sparta::bind(getRoot()->getChildAs<sparta::Port>("rob.ports.Rob_cmt_inst_out"),
                     getRoot()->getChildAs<sparta::Port>("renaming_stage.ports.Rob_cmt_inst_in"));


        /* Credits */
        // renaming stage -> frontend
        sparta::bind(getRoot()->getChildAs<sparta::Port>("renaming_stage.ports.renaming_preceding_credit_out"),
                     getRoot()->getChildAs<sparta::Port>("perfect_frontend.ports.backend_fetch_credit_in"));

        // rob -> renaming stage
        sparta::bind(getRoot()->getChildAs<sparta::Port>("rob.ports.rob_preceding_credit_out"),
                     getRoot()->getChildAs<sparta::Port>("renaming_stage.ports.rob_renaming_credit_in"));

        // dispatch stage -> renaming stage
        sparta::bind(getRoot()->getChildAs<sparta::Port>("dispatch_stage.ports.dispatch_preceding_credit_out"),
                     getRoot()->getChildAs<sparta::Port>("renaming_stage.ports.following_renaming_credit_in"));

        // separate rs -> dispatch stage
        for (auto& func_pair: getFuncMap()) {
            sparta::bind(getRoot()->getChildAs<sparta::Port>("rs_"+func_pair.first+".ports.reservation_preceding_credit_out"),
                         getRoot()->getChildAs<sparta::Port>("dispatch_stage.ports."+func_pair.first+"_rs_dispatch_credit_in"));
        }

        // write back -> separate rs
        for (auto& func_pair: getFuncMap()) {
            sparta::bind(getRoot()->getChildAs<sparta::Port>("write_back_stage.ports."+ func_pair.first + "_rs_credit_out"),
                         getRoot()->getChildAs<sparta::Port>("rs_"+func_pair.first+".ports.following_reservation_credit_in"));
        }

        // precedes
        
    }

    void Simulation::test() {}
} // namespace Yihai
