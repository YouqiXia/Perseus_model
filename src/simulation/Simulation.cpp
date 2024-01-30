#include "Simulation.hpp"
#include "sparta/ports/Port.hpp"
#include "sparta/utils/StringUtils.hpp"

#include <memory>
#include <cstdint>

#include "olympia/OlympiaAllocators.hpp"

#include "FuncUnits.hpp"

namespace TimingModel {

    // third party units
    std::unique_ptr<OlympiaAllocators> global_allocators;

    // Simulation
    Simulation::Simulation(sparta::Scheduler& Sched,
                           const CommandLineData cmd_data,
                           VAR::DRAMinput DRAMinput) :
        sparta::app::Simulation("Yihai_test", &Sched),
        is_elf_workload_(cmd_data.is_elf_workload),
        workload_(cmd_data.workload),
        dram_input_(DRAMinput),
        instruction_limit_(cmd_data.instruction_limit)
    {
        sparta::StartupEvent(getRoot(), CREATE_SPARTA_HANDLER(Simulation, test));
    }

    Simulation::~Simulation() {
        getRoot()->enterTeardown();
    }

    void Simulation::buildTree_() {

        global_allocators = std::make_unique<OlympiaAllocators>(getRoot());
        getRoot()->addExtensionFactory(TimingModel::TopoExtensions::name,
                                        [&]()->sparta::TreeNode::ExtensionsBase * {return new TimingModel::TopoExtensions();});
        auto module_topo = TimingModel::getCommonTopology(getRoot(), "processor");
        buildHierarchicalTopology_(module_topo, getRoot());
        setFuCfgFromExtensions(getRoot());
    }

    void Simulation::configureTree_()
    {
        // In TREE_CONFIGURING phase
        // Configuration from command line is already applied

        sparta::ParameterBase* max_instrs =
            getRoot()->getChildAs<sparta::ParameterBase>("rob.params.num_insts_to_retire");

        // Safely assign as string for now in case parameter type changes.
        // Direct integer assignment without knowing parameter type is not yet available through C++ API
        if(instruction_limit_ != 0){
            max_instrs->setValueFromString(sparta::utils::uint64_to_str(instruction_limit_));
        }
    }

    void Simulation::bindTree_() {
        auto binding_topo = TimingModel::getBindingTopology(getRoot());
        bindBindingTopology_(binding_topo);
    }

    void Simulation::test() {}

    void Simulation::buildHierarchicalTopology_(TopoExtensions::CommonTopoType& topo, sparta::TreeNode* parent){
        for(auto node_name : topo ){
            try {
                if (parent->getExtension("topo_extensions")->getParameters()->getParameter(node_name[1]) != nullptr) {
                    auto next_topo = parent->getExtension("topo_extensions")->getParameters()->getParameter(
                                    node_name[1])->getValueAs<TopoExtensions::CommonTopoType>();
                    buildHierarchicalTopology_(next_topo, parent);
                }
            } catch(std::exception e) {
                if (node_name[1] == "") {
                    return;
                }
                auto node_tmp = new sparta::ResourceTreeNode(parent,
                                                             node_name[0],
                                                             sparta::TreeNode::GROUP_NAME_NONE,
                                                             sparta::TreeNode::GROUP_IDX_NONE,
                                                             node_name[0],
                                                             resource_map_factory_[node_name[1]]);
                to_delete_.emplace_back(node_tmp);
                if(node_name[1].compare("perfect_frontend") == 0){
                    node_tmp->getParameterSet()->getParameter("input_file")->setValueFromString(workload_);
                    if (is_elf_workload_) {
                        node_tmp->getParameterSet()->getParameter("insn_gen_type")->setValueFromString("spike");
                    }
                }
            }
        }
    }

    void Simulation::bindBindingTopology_(TopoExtensions::BindingTopology& topo){
        for(auto i=0u; i<topo.size(); ){
            sparta::bind(getRoot()->getChildAs<sparta::Port>(topo[i]),
                        getRoot()->getChildAs<sparta::Port>(topo[i+1]));
            i += 2;
        }
    }

} // namespace Yihai
