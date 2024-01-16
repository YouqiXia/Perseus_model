#include "simulation/Simulation.hpp"

#include "sparta/app/SimulationConfiguration.hpp"
#include "sparta/app/FeatureConfiguration.hpp"
#include "sparta/app/CommandLineSimulator.hpp"

#include "basic/Inst.hpp"
#include "simulation/variable.hpp"

// User-friendly usage that correspond with sparta::app::CommandLineSimulator
// options
const char USAGE[] =
    "Usage:\n"
    "    [-i insts] [-r RUNTIME] [--show-tree] [--show-dag]\n"
    "    [-p PATTERN VAL] [-c FILENAME]\n"
    "    [-l PATTERN CATEGORY DEST]\n"
    "    [-h,--help] <workload [stf trace or JSON]>\n"
    "    <--DRAMconfig DRAMsim3 configfile> <--DRAMoutdir DRAM output diretory>"
    "\n";


sparta::Scheduler* g_scheduler;
int main(int argc, char **argv) {
    std::string workload;
    std::string DRAMconfig;
    std::string DRAMoutdir;
    const char * WORKLOAD = "workload";
    sparta::Scheduler scheduler;
    g_scheduler = &scheduler;
    sparta::app::DefaultValues DEFAULTS;
    DEFAULTS.auto_summary_default = "off";
    DEFAULTS.arch_arg_default = "extensions";
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
             "Specifies the instruction workload (trace, JSON)")
            ("DRAMconfig",
             sparta::app::named_value<std::string>("\"\"", &DRAMconfig),
             "Specifies the path of DRAM config file")
            ("DRAMoutdir",
             sparta::app::named_value<std::string>("\"\"", &DRAMoutdir),
             "Specifies the path of DRAM output directory");

        // Add any positional command-line options
        po::positional_options_description& pos_opts = cls.getPositionalOptions();
        pos_opts.add(WORKLOAD, -1);

        // Parse command line options and configure simulator
        int err_code = 0;
        if(!cls.parse(argc, argv, err_code)){
            return err_code; // Any errors already printed to cerr
        }

        //Check DRAM config file
        if (DRAMconfig.empty() || DRAMoutdir.empty()){
            std::cerr << "ERROR: Missing DRAM files." << std::endl;
            std::cerr << USAGE;
            return -1;
        }

        auto& vm = cls.getVariablesMap();

        VAR::DRAMinput dram_input;
        dram_input.DRAMconfig = vm["DRAMconfig"].as<std::string>();
        dram_input.DRAMoutdir = vm["DRAMoutdir"].as<std::string>();

        if (vm.count("run") != 0) {
            // Create the simulator

            TimingModel::Simulation sim(scheduler,
                                        workload,
                                        dram_input);

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