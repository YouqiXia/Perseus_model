#ifndef YIHAI_SIMULATION_HPP_
#define YIHAI_SIMULATION_HPP_

#include "sparta/app/Simulation.hpp"
#include "sparta/simulation/ResourceFactory.hpp"

#include <string>

#include "olympia/OlympiaAllocators.hpp"
#include "sparta/simulation/ParameterSet.hpp"
#include "sparta/utils/Utils.hpp"
#include "basic/Instruction.hpp"
#include "ExtensionsCfg.hpp"
#include "ResourceMapFactory.hpp"
#include "variable.hpp"

struct CommandLineData {
    bool is_elf_workload = false;
    std::string workload = "";
    uint64_t instruction_limit = 0;
};

namespace TimingModel {

    class Simulation : public sparta::app::Simulation {
    public:
        Simulation(sparta::Scheduler& scheduler,
                   const CommandLineData cmd_data,
                   VAR::DRAMinput DRAMinput);

        virtual ~Simulation();

        void test();

    private:
        void buildTree_() override;

        //! Configure the tree and apply any last minute parameter changes
        void configureTree_() override;

        //! The tree is now configured, built, and instantiated.  We need
        //! to bind things together.
        void bindTree_() override;

        void buildHierarchicalTopology_(TopoExtensions::CommonTopoType&, sparta::TreeNode*);

        void bindBindingTopology_(TopoExtensions::BindingTopology&);

    private:
        bool is_elf_workload_;

        std::string workload_;

        VAR::DRAMinput dram_input_;

        //! Instruction limit (set up -i option on command line)
        const uint64_t instruction_limit_;

        ResourceMapFactory resource_map_factory_;
    };
}

#endif // YIHAI_SIMULATION_HPP_