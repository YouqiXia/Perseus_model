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

namespace TimingModel {
    extern std::map<std::string, sparta::ResourceFactoryBase*> factory_map;

    const uint64_t max_run_time = 200;

    class Simulation : public sparta::app::Simulation {
    public:
        Simulation(sparta::Scheduler& scheduler,
                   const std::string workload);

        virtual ~Simulation();

        void test();

    private:
        void buildTree_() override;

        //! Configure the tree and apply any last minute parameter changes
        void configureTree_() override;

        //! The tree is now configured, built, and instantiated.  We need
        //! to bind things together.
        void bindTree_() override;

        void BuildFuncRelatives_();

        void buildModuleTopology_(TopoExtensions::ModuleTopology&, sparta::TreeNode*);

        void bindBindingTopology_(TopoExtensions::BindingTopology&);

    private:
        std::string workload_;

        ResourceMapFactory resource_map_factory_;
    };
}

#endif // YIHAI_SIMULATION_HPP_