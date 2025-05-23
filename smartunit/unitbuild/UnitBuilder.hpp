#pragma once

#include <sparta/simulation/ResourceTreeNode.hpp>
#include <sparta/simulation/TreeNode.hpp>

#include <string>
#include <vector>
#include <map>

namespace TimingModel {

class UnitBuilder {
public:
    struct Params {
        std::string unit_conf_path;
        std::string unit_bind_path;
        std::string port_conf_path;
        std::string default_params_path;
    };

    UnitBuilder(const std::string &conf_dir);
    UnitBuilder(const UnitBuilder::Params &params);
    std::vector<sparta::TreeNode *> build(sparta::RootTreeNode *root);
    void bind(sparta::RootTreeNode *root);

private:
    void bind(std::vector<sparta::ResourceTreeNode *> &unitset1, int beg1, int end1,
              std::vector<sparta::ResourceTreeNode *> &unitset2, int beg2, int end2,
              const std::vector<std::vector<int>> &matrix,
              const std::vector<std::string> &ports_bind, bool in_order);

    UnitBuilder::Params params_;

    using UnitNameMap = std::map<std::string, std::vector<sparta::ResourceTreeNode *>>;
    std::map<int, UnitNameMap> final_unit_instances_map_;
};

}