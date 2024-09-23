//
// Created by yzhang on 9/1/24.
//

#ifndef MODEL_UNITFACTORY_HPP
#define MODEL_UNITFACTORY_HPP

#include "simulation/ResourceMapFactory.hpp"
#include "sparta/simulation/ResourceTreeNode.hpp"
#include "sparta/simulation/TreeNode.hpp"

#include <any>

namespace PybindInterface {
    class UnitsSet {
    public:
        typedef std::string UnitName;

        typedef std::string PortName;
        typedef std::string PortDirection;

        typedef std::string ParamsName;
        typedef bool ParamsBool;
        typedef uint64_t ParamsInt;
        typedef std::string ParamsString;
        typedef std::vector<std::string> ParamsVectorString;

        typedef std::string UnitInfoTitle;
        typedef std::string UnitSubInfoTitle;
        typedef std::unordered_map<PortDirection, std::vector<PortName>> PortInfo;
        typedef std::unordered_map<ParamsName, sparta::ParameterBase*> ParamsInfo;

        typedef struct {
            PortInfo ports_info;
            ParamsInfo params_info;
        } UnitStatistics;

        typedef std::unordered_map<UnitName, UnitStatistics> UnitInfo;

        UnitsSet() : resource_map() {
            sparta::Scheduler scheduler;
            sparta::Clock clk("clock", &scheduler);
            sparta::RootTreeNode dummy_node("dummy_rtn");
            dummy_node.setClock(&clk);
            dummy_node.enterConfiguring();
            
            for (auto factory_base_pair: resource_map.GetResourceFactory()) {
                resource_nodes.emplace_back(new sparta::ResourceTreeNode{&dummy_node,
                                                                         factory_base_pair.first,
                                                                         sparta::TreeNode::GROUP_NAME_NONE,
                                                                         sparta::TreeNode::GROUP_IDX_NONE,
                                                                         factory_base_pair.first,
                                                                         factory_base_pair.second});
            }
            dummy_node.enterFinalized();

            for (auto resource_node: resource_nodes) {
                std::string node_name = resource_node->getName();
                unit_names.emplace_back(node_name);
                auto in_port_map = resource_node->getResourceAs<sparta::Unit>()->getPortSet()->
                        getPorts(sparta::Port::Direction::IN);
                for (auto port_name_pair: in_port_map) {
                    unit_ports.emplace_back(port_name_pair.first);
                    units_info[node_name].ports_info["in_ports"].push_back(port_name_pair.first);
                }

                auto out_port_map = resource_node->getResourceAs<sparta::Unit>()->getPortSet()->
                        getPorts(sparta::Port::Direction::OUT);
                for (auto port_name_pair: out_port_map) {
                    unit_ports.emplace_back(port_name_pair.first);
                    units_info[node_name].ports_info["out_ports"].push_back(port_name_pair.first);
                }

                auto param_set = resource_node->getParameterSet();
                for (auto param: *param_set) {
                    units_info[node_name].params_info[param->getName()] = param;
                }
            }

            dummy_node.enterTeardown();

        }

        UnitInfo GetUnitsInfo() const {
            return units_info;
        }

        std::vector<UnitName> GetNameSet() const {
            return unit_names;
        }

        std::vector<UnitName> GetPortSet() const {
            return unit_ports;
        }

    private:
        TimingModel::ResourceMapFactory resource_map;
        std::vector<sparta::ResourceTreeNode*> resource_nodes;
        UnitInfo units_info;
        std::vector<UnitName> unit_names;
        std::vector<PortName> unit_ports;
    };
}



#endif //MODEL_UNITFACTORY_HPP
