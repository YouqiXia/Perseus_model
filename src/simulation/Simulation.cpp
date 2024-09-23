#include "Simulation.hpp"
#include "sparta/ports/Port.hpp"
#include "sparta/utils/StringUtils.hpp"

#include <memory>
#include <cstdint>

namespace TimingModel {

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
        std::ifstream json_file(cmd_data.json_config);
        nlohmann::json json_data;
        json_file >> json_data;
        json_config_ = json_data;
        sparta::StartupEvent(getRoot(), CREATE_SPARTA_HANDLER(Simulation, test));
    }

    Simulation::~Simulation() {
        getRoot()->enterTeardown();
    }

    void Simulation::buildTree_() {
        buildHierarchicalTopology_(json_config_["hierarchy"], getRoot());
    }

    void Simulation::configureTree_()
    {}

    void Simulation::bindTree_() {
        bindBindingTopology_(json_config_["binding"], getRoot());
    }

    void Simulation::test() {}

    void Simulation::buildHierarchicalTopology_(const nlohmann::json &json, sparta::TreeNode* parent) {
        if (json.is_object()) {
            for (auto it = json.begin(); it != json.end(); ++it) {
                if (it.key() == "info" || it.key() == "root") {
                    buildHierarchicalTopology_(it.value(), parent);
                } else {
                    auto *node = new sparta::TreeNode(parent, it.key(), "hierarchy node");
                    to_delete_.emplace_back(node);
                    buildHierarchicalTopology_(it.value(), node);
                }
            }
        } else if (json.is_array()) {
            for (const auto &units_topo: json) { // units_map
                for (auto units_pair = units_topo.begin();
                     units_pair != units_topo.end(); ++units_pair) { // unit type - unit name pair
                    if (units_pair.value() == SelfAllocatorsUnit::name) {
                        continue;
                    }
                    for (auto units_instance = units_pair.value().begin();
                         units_instance != units_pair.value().end(); ++units_instance) { // instance name - params pair
                        const std::string instance_name = units_instance.key();
                        auto node = new sparta::ResourceTreeNode(parent,
                                                                 instance_name,
                                                                 sparta::TreeNode::GROUP_NAME_NONE,
                                                                 sparta::TreeNode::GROUP_IDX_NONE,
                                                                 instance_name,
                                                                 resource_map_factory_[units_pair.key()]);
                        to_delete_.emplace_back(node);
                        for (auto params_pair = units_instance.value().begin();
                             params_pair != units_instance.value().end(); ++params_pair) {// params name - params data
                            if (params_pair.value().is_array()) {
                                if (!params_pair.value().empty()) {
                                    node->getParameterSet()->getParameter(params_pair.key())->
                                            setValueFromStringVector(
                                            params_pair.value().get < std::vector < std::string >> ());
                                }
                            } else if (params_pair.value().is_string()) {
                                node->getParameterSet()->getParameter(params_pair.key())->
                                        setValueFromString(params_pair.value().get<std::string>());
                            } else {
                                node->getParameterSet()->getParameter(params_pair.key())->
                                        setValueFromString(params_pair.value().dump());
                            }
                        }
                        if (units_pair.key().compare("perfect_frontend") == 0) {
                            node->getParameterSet()->getParameter("input_file")->setValueFromString(workload_);
                            node->getParameterSet()->getParameter("is_config")->setValueFromString("false");
                            if (is_elf_workload_) {
                                node->getParameterSet()->getParameter("insn_gen_type")->setValueFromString("spike");
                            }
                        }
                    }
                }
            }
        }
    }

    void Simulation::bindBindingTopology_(const nlohmann::json &json, sparta::TreeNode*){
        for (const auto &binding_pair: json) { // units_map
            sparta::bind(getRoot()->getChildAs<sparta::Port>(binding_pair["source"]),
                        getRoot()->getChildAs<sparta::Port>(binding_pair["target"]));
        }
    }

} // namespace Yihai
