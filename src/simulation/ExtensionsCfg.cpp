#include "ExtensionsCfg.hpp"

namespace TimingModel {

    TopoExtensions::CommonTopoType getCommonTopology(sparta::TreeNode * node, std::string topo_name)
    {
        auto topo_extension           = node->getExtension("topo_extensions");
        auto topo_extension_params    = sparta::notNull(topo_extension)->getParameters();
        auto common_topology_param = sparta::notNull(topo_extension_params)->getParameter(topo_name);
        return sparta::notNull(common_topology_param)->getValueAs<TopoExtensions::CommonTopoType>();
    }

    TopoExtensions::BindingTopology getBindingTopology(sparta::TreeNode * node)
    {
        auto topo_extension           = node->getExtension("topo_extensions");
        auto topo_extension_params    = sparta::notNull(topo_extension)->getParameters();
        auto binding_topology_param = sparta::notNull(topo_extension_params)->getParameter("binding_topology");
        return sparta::notNull(binding_topology_param)->getValueAs<TopoExtensions::BindingTopology>();
    }
}