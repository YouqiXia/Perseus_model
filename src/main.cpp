#include "simulation/Simulation.hpp"

#include "sparta/app/SimulationConfiguration.hpp"
#include "sparta/app/FeatureConfiguration.hpp"
#include "sparta/app/CommandLineSimulator.hpp"

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

// User-friendly usage that correspond with sparta::app::CommandLineSimulator
// options
const char USAGE[] =
    "Usage:\n"
    "    [-i insts] [-r RUNTIME] [--show-tree] [--show-dag]\n"
    "    [-p PATTERN VAL] [-c FILENAME]\n"
    "    [-l PATTERN CATEGORY DEST]\n"
    "    [-h,--help] <workload [stf trace or JSON]>\n"
    "\n";

int main(int argc, char **argv) {
    std::string workload;
    const char * WORKLOAD = "workload";

    sparta::app::DefaultValues DEFAULTS;
    DEFAULTS.auto_summary_default = "off";
    DEFAULTS.arch_search_dirs = {"arches"}; // Where --arch will be resolved by default

    // try/catch block to ensure proper destruction of the cls/sim classes in
    // the event of an error
    try{
        // Helper class for parsing command line arguments, setting up the
        // simulator, and running the simulator. All of the things done by this
        // classs can be done manually if desired. Use the source for the
        // CommandLineSimulator class as a starting point
        sparta::app::CommandLineSimulator cls(USAGE, DEFAULTS);
        auto& app_opts = cls.getApplicationOptions();
        app_opts.add_options()
            ("run",
             "run the simulation")
            (WORKLOAD,
             sparta::app::named_value<std::string>(WORKLOAD, &workload),
             "Specifies the instruction workload (trace, JSON)");

        // Add any positional command-line options
        po::positional_options_description& pos_opts = cls.getPositionalOptions();
        pos_opts.add(WORKLOAD, -1);

        // Parse command line options and configure simulator
        int err_code = 0;
        if(!cls.parse(argc, argv, err_code)){
            return err_code; // Any errors already printed to cerr
        }

        auto& vm = cls.getVariablesMap();

        if (vm.count("run") != 0) {
            // Create the simulator
            sparta::Scheduler scheduler;
            TimingModel::Simulation sim(scheduler);

            cls.populateSimulation(&sim);

            cls.runSimulator(&sim);

            cls.postProcess(&sim);
        }
    }catch(...){
        // Could still handle or log the exception here
        throw;
    }

    return 0;
}