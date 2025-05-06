#pragma once

#include <sparta/simulation/ResourceTreeNode.hpp>
#include <sparta/simulation/TreeNode.hpp>

#include "smartunit/registered/UnitRegister.hpp"

#include <any>

namespace PybindInterface {

class UnitNode {
public:
    UnitNode() {}

    TimingModel::RegisterType m_type;
    std::string m_name;
    std::vector<std::string> m_params;
    std::vector<std::string> m_inports;
    std::vector<std::string> m_outports;
};

class UnitsSet {
public:
    UnitsSet() {
        sparta::Scheduler scheduler;
        sparta::Clock clk("clock", &scheduler);
        sparta::RootTreeNode dummy_node("dummy_rtn");
        dummy_node.setClock(&clk);
        dummy_node.enterConfiguring();
        
        for (auto &[type, factorys_map] : TimingModel::UnitRegister::instance().getAllFactory()) {
            for (auto &factory_base_pair : factorys_map) {
                m_resource_nodes.emplace_back(new sparta::ResourceTreeNode{&dummy_node,
                                                                        factory_base_pair.first,
                                                                        sparta::TreeNode::GROUP_NAME_NONE,
                                                                        sparta::TreeNode::GROUP_IDX_NONE,
                                                                        factory_base_pair.first,
                                                                        factory_base_pair.second});
                m_unitTypes.push_back(type);
            }
        }
        dummy_node.enterFinalized();

        m_unitNodes.resize(m_resource_nodes.size());
        size_t index = 0;
        for (auto resource_node: m_resource_nodes) {
            m_unitNodes[index].m_type = m_unitTypes[index];
            m_unitNodes[index].m_name = resource_node->getName();
            auto &in_port_map = resource_node->getResourceAs<sparta::Unit>()->getPortSet()->
                    getPorts(sparta::Port::Direction::IN);
            for (auto &port_name_pair: in_port_map) {
                m_unitNodes[index].m_inports.emplace_back(port_name_pair.first);
            }

            auto &out_port_map = resource_node->getResourceAs<sparta::Unit>()->getPortSet()->
                    getPorts(sparta::Port::Direction::OUT);
            for (auto &port_name_pair: out_port_map) {
                m_unitNodes[index].m_outports.emplace_back(port_name_pair.first);
            }

            auto param_set = resource_node->getParameterSet();
            for (auto param: *param_set) {
                m_unitNodes[index].m_params.emplace_back(param->getName());
            }

            index++;
        }

        dummy_node.enterTeardown();

    }

    ~UnitsSet() {
        for (auto resource : m_resource_nodes) {
            delete resource;
        }
    }

    const std::vector<UnitNode> &getUnitNodes() const {
        return m_unitNodes;
    }

private:
    std::vector<sparta::ResourceTreeNode*> m_resource_nodes;
    std::vector<TimingModel::RegisterType> m_unitTypes;
    std::vector<UnitNode> m_unitNodes;
};

}
