#include "ExtensionsCfg.hpp"

namespace TimingModel {

    TopoExtensions::ModuleTopology getModuleTopology(sparta::TreeNode * node)
    {
        auto topo_extension           = node->getExtension("topo_extensions");
        auto topo_extension_params    = sparta::notNull(topo_extension)->getParameters();
        auto module_topology_param = sparta::notNull(topo_extension_params)->getParameter("module_topology");
        return sparta::notNull(module_topology_param)->getValueAs<TopoExtensions::ModuleTopology>();
    }

    TopoExtensions::BindingTopology getBindingTopology(sparta::TreeNode * node)
    {
        auto topo_extension           = node->getExtension("topo_extensions");
        auto topo_extension_params    = sparta::notNull(topo_extension)->getParameters();
        auto binding_topology_param = sparta::notNull(topo_extension_params)->getParameter("binding_topology");
        return sparta::notNull(binding_topology_param)->getValueAs<TopoExtensions::BindingTopology>();
    }
    TopoExtensions::FuMap getFuMap(sparta::TreeNode * node){
        auto topo_extension           = node->getExtension("topo_extensions");
        auto topo_extension_params    = sparta::notNull(topo_extension)->getParameters();
        auto fu_map_param = sparta::notNull(topo_extension_params)->getParameter("fu_map");
        return sparta::notNull(fu_map_param)->getValueAs<TopoExtensions::FuMap>();
    } 
    TopoExtensions::FuCreditMap getFuCreditMap(sparta::TreeNode * node){
        auto topo_extension           = node->getExtension("topo_extensions");
        auto topo_extension_params    = sparta::notNull(topo_extension)->getParameters();
        auto fu_credit_map_param = sparta::notNull(topo_extension_params)->getParameter("fu_credit_map");
        return sparta::notNull(fu_credit_map_param)->getValueAs<TopoExtensions::FuCreditMap>();
    }
}