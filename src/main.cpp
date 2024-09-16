#include <string>

#include "simulation/Simulation.hpp"

#include "sparta/utils/File.hpp"
#include "sparta/simulation/ParameterTree.hpp"
#include "sparta/simulation/TreeNode.hpp"

#include "sparta/app/SimulationConfiguration.hpp"
#include "sparta/app/FeatureConfiguration.hpp"
#include "sparta/app/CommandLineSimulator.hpp"

#include "sparta/kernel/SleeperThread.hpp"

#include "basic/Inst.hpp"
#include "simulation/variable.hpp"

// for test
#include "simulation/ResourceMapFactory.hpp"

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

//    try{
        // Helper class for parsing command line arguments, setting up the
        // simulator, and running the simulator. All of the things done by this
        // classs can be done manually if desired. Use the source for the
        // CommandLineSimulator class as a starting point
        sparta::app::CommandLineSimulator cls(USAGE, DEFAULTS);
        auto& app_opts = cls.getApplicationOptions();
        app_opts.add_options()
            ("debug",
             "model will stalls in the debug mode")
            ("json",
             sparta::app::named_value<std::string>("unit", &cmd_data.json_config),
             "Specifies the units should be to build")
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


        if (vm.count("debug") != 0) {
//            std::vector<std::string> arch_search_paths {"arches"};
//            sparta::TreeNode dummy("dummy", "dummy");
//            std::string found_filename = sparta::utils::findArchitectureConfigFile(arch_search_paths, cmd_data.unit_factory);
//            sparta::ConfigParser::YAML param_file(found_filename, arch_search_paths);
//            param_file.allowMissingNodes(true);
//            param_file.consumeParameters(&dummy);
//            auto param_node = param_file.getParameterTree().getRoot();
//            PybindInterface::UnitsSet unit_set;

//            sparta::Scheduler scheduler;
//            sparta::Clock clk("clock", &scheduler);
//            sparta::RootTreeNode dummy_node("dummy_rtn");
//            dummy_node.setClock(&clk);
//            dummy_node.enterConfiguring();
//            TimingModel::ResourceMapFactory resource_map;
//            // TODO: should be modified
//            TimingModel::OlympiaAllocators allocator{&dummy_node};
//            sparta::ResourceTreeNode* resource_node = new sparta::ResourceTreeNode{&dummy_node,
//                                                                     "perfect_alu",
//                                                                     sparta::TreeNode::GROUP_NAME_NONE,
//                                                                     sparta::TreeNode::GROUP_IDX_NONE,
//                                                                     "perfect_alu",
//                                                                     resource_map["perfect_alu"]};
//            std::cout << resource_node->getParameterSet()->getParameter("haha")->getDefaultAsString() << std::endl;
//            resource_node->getParameterSet()->getParameter("haha")->setValueFromStringVector({"fdv", "is", "my", "son"});
//            std::cout << resource_node->getParameterSet()->getParameter("haha")->getValueAsString() << std::endl;
//            for (auto param: *resource_node->getParameterSet()) {
//                std::cout << param->getName() << ", ";
//                std::cout << param->getDefaultAsString() << ", ";
//                std::cout << param->getTypeName() << ", ";
//                std::cout << std::endl;
//            }
//            dummy_node.enterFinalized();
//            dummy_node.enterTeardown();
        } else {
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

//    }catch(...){
//        // Could still handle or log the exception here
//        throw;
//    }

    return 0;
}