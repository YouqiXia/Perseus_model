#include "simulation/Simulation.hpp"

#include "sparta/app/SimulationConfiguration.hpp"
#include "sparta/app/FeatureConfiguration.hpp"

#include "basic/Instruction.hh"
#include "interface/backend/IRob.hh"
#include "interface/backend/IFreeList.hh"
#include "interface/backend/IPhysicalReg.hh"
#include "interface/backend/IReservationStation.hh"
#include "interface/backend/IRenamingTable.hh"
#include "interface/backend/IBusyTable.hh"
#include "interface/backend/IDispatchQueue.hh"

#include "interface/frontend/IInstQueue.hh"

#include "interface/functionunit/IperfectAlu.hh"
#include "interface/functionunit/IperfectLsu.hh"

int main() {
    sparta::Scheduler sched;
    TimingModel::Simulation sim(sched);

    // more configuration
    sparta::app::FeatureConfiguration feature_config_;
    if (!feature_config_.isFeatureValueSet("simdb")) {
        feature_config_.setFeatureValue("simdb", 0);
    }
    sim.setFeatureConfig(&feature_config_);

    sparta::app::SimulationConfiguration sim_config_;

    char* tmp_string = "haha";

    sim.configure(int(0),
                   &tmp_string,
                   &sim_config_,
                   false);

    sim.buildTree();

    sim.configureTree();

    sim.finalizeTree();

    sim.finalizeFramework();

    sim.Test();

    return 0;
}