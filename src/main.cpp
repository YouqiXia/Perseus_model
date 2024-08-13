#include "simulation/Simulation.hpp"

#include "sparta/app/SimulationConfiguration.hpp"
#include "sparta/app/FeatureConfiguration.hpp"
#include "sparta/app/CommandLineSimulator.hpp"

#include "sparta/kernel/SleeperThread.hpp"

#include "basic/Inst.hpp"
#include "simulation/variable.hpp"

// User-friendly usage that correspond with sparta::app::CommandLineSimulator
// options
const char USAGE[] =
    "Usage:\n"
    "    [-i insts] [-r RUNTIME] [--show-tree] [--show-dag]\n"
    "    [-p PATTERN VAL] [-c FILENAME]\n"
    "    [-l PATTERN CATEGORY DEST]\n"
    "    [-h,--help] <workload [stf trace or JSON], elf [elf]>\n"
    "    <--DRAMconfig configfile> <--DRAMoutdir DRAM_output_diretory>"
    "\n";

int main(int argc, char **argv) {
    CommandLineData cmd_data;
    const char * WORKLOAD = "workload";
    std::string DRAMconfig;
    std::string DRAMoutdir;
    uint64_t ilimit = 0;

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
            ("elf",
             sparta::app::named_value<std::string>("elf", &cmd_data.workload),
             "Specifies the instruction workload (elf)")
            (WORKLOAD,
             sparta::app::named_value<std::string>(WORKLOAD, &cmd_data.workload),
             "Specifies the instruction workload (trace, JSON)")
            ("DRAMconfig",
             sparta::app::named_value<std::string>("\"\"", &DRAMconfig),
             "Specifies the path of DRAM config file")
            ("DRAMoutdir",
             sparta::app::named_value<std::string>("\"\"", &DRAMoutdir),
             "Specifies the path of DRAM output directory")
            ("instruction-limit,i",
             sparta::app::named_value<uint64_t>("LIMIT", &cmd_data.instruction_limit)->default_value(ilimit),
             "Limit the simulation to retiring a specific number of instructions. 0 (default) "
             "means no limit. If -r is also specified, the first limit reached ends the simulation",
             "End simulation after a number of instructions. Note that if set to 0, this may be "
             "overridden by a node parameter within the simulator");

        // Add any positional command-line options
        po::positional_options_description& pos_opts = cls.getPositionalOptions();
        pos_opts.add(WORKLOAD, -1);

        // Parse command line options and configure simulator
        int err_code = 0;
        if(!cls.parse(argc, argv, err_code)){
            return err_code; // Any errors already printed to cerr
        }

        auto& vm = cls.getVariablesMap();

        VAR::DRAMinput dram_input;
        if (DRAMconfig.empty() || DRAMoutdir.empty()){
            dram_input.DRAMconfig = "";
            dram_input.DRAMoutdir = "";
        } else {
            dram_input.DRAMconfig = vm["DRAMconfig"].as<std::string>();
            dram_input.DRAMoutdir = vm["DRAMoutdir"].as<std::string>();
        }

        if (vm.count("run") != 0) {

            if (vm.count("elf") != 0) {
                cmd_data.is_elf_workload = true;
            }

            // Create the simulator
            sparta::Scheduler scheduler;
            TimingModel::Simulation sim(scheduler,
                                        cmd_data,
                                        dram_input);  // run for ilimit instructions

            sparta::SleeperThread::disableForever();
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