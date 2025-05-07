#include "UnitBuilder.hpp"

#include <nlohmann/json.hpp>

#include <fstream>

#include "smartunit/registered/UnitRegister.hpp"

namespace TimingModel {

static std::pair<std::string, std::string> spilt_unitkey(const std::string &unitkey, char deli) {
    size_t pos = unitkey.find(deli);
    std::pair<std::string, std::string> result;

    if (pos != std::string::npos) {
        result.first = unitkey.substr(0, pos);
        result.second = unitkey.substr(pos + 1, unitkey.size() - pos - 1);
    }
    return result;
}

UnitBuilder::UnitBuilder(const UnitBuilder::Params &params) : params_(params) {}

std::vector<sparta::TreeNode *> UnitBuilder::build(sparta::RootTreeNode *root) {
    /* Build tree. */
    // std::cout << "build beg" << std::endl;
    std::vector<sparta::TreeNode *> nodes; /* All tree node. */
    nlohmann::json unit_conf_data;
    {
        std::ifstream f(params_.unit_conf_path);
        unit_conf_data = nlohmann::json::parse(f);
    }

    nlohmann::json default_params_data;
    {
        std::ifstream f(params_.default_params_path);
        default_params_data = nlohmann::json::parse(f);
    }

    auto &unit_instances = unit_conf_data["unit_instances"];
    auto &unit_instance_params = unit_conf_data["unit_instance_params"];
    int num = 0;
    int core_idx = 0;
    int cluster_idx = 0;
    std::string modulename;
    std::string unitname;

    auto &all_factory = UnitRegister::instance().getAllFactory();

    std::unordered_map<RegisterType, sparta::TreeNode *> module_map;

    for (const auto &[key, value] : unit_instances.items()) {
        num = value[0].get<int>();
        core_idx = value[1].get<int>();
        cluster_idx = value[2].get<int>();
        auto split_pair = spilt_unitkey(key, '.');
        modulename = split_pair.first;
        unitname = split_pair.second;

        /* Fill unit_instances_map_ */
        unit_instances_map_.emplace(key, UnitInstance());
        unit_instances_map_[key].num = num;

        auto &params_json = unit_instance_params[key]; /* Get params json node. */

        auto register_type = str_to_registertype(modulename);
        sparta::TreeNode *parent{nullptr};
        if (module_map.count(register_type) == 0) {
            parent = new sparta::TreeNode(root, modulename, "module");
            module_map.emplace(register_type, parent);
            nodes.push_back(parent);
        } else {
            parent = module_map.at(register_type);
        }

        auto &factory_map = all_factory.at(register_type);
        auto factory = factory_map.at(unitname);

        for (int i = 0; i < num; ++i) {
            std::string instance_name = unitname + "_" + std::to_string(cluster_idx)
                                        + "_" + std::to_string(core_idx)
                                        + "_" + std::to_string(i);
            // std::cout << "build: " << key << " " << instance_name << std::endl;
            auto node = new sparta::ResourceTreeNode(parent, instance_name, sparta::TreeNode::GROUP_NAME_NONE,
                                            sparta::TreeNode::GROUP_IDX_NONE,
                                            instance_name, factory);
            nodes.push_back(node);
            unit_instances_map_[key].nodes.push_back(node);

            /* Default params setup. */
            if (default_params_data.contains(key)) {
                auto &key_default_params = default_params_data[key];
                for (const auto &[param_key, param_value] : key_default_params.items()) {
                    node->getParameterSet()->getParameter(param_key)->setValueFromString(param_value.get<std::string>());
                    // std::cout << "gparam: " << param_key << " " << param_value.get<std::string>() << std::endl;
                }
            }

            /* Local params setup. */
            if (not params_json.empty()) {
                for (const auto &[param_key, param_value] : params_json.items()) {
                    if (not node->getParameterSet()->hasParameter(param_key)) continue;
                    node->getParameterSet()->getParameter(param_key)->setValueFromString(param_value[i].get<std::string>());
                    // std::cout << "lparam: " << param_key << " " << param_value[i].get<std::string>() << std::endl;
                }
            }
        }
    }
    // std::cout << "build end" << std::endl;
    return nodes;
}

void UnitBuilder::bind(sparta::RootTreeNode *root) {
    nlohmann::json port_conf_data;
    nlohmann::json unit_bind_data;
    /* Load port conf. */
    {
        std::ifstream f(params_.port_conf_path);
        port_conf_data = nlohmann::json::parse(f);
    }
    /* Load unit bind. */
    {
        std::ifstream f(params_.unit_bind_path);
        unit_bind_data = nlohmann::json::parse(f);
    }

    std::vector<std::vector<int>> matrix;
    std::vector<std::string> ports_bind;
    auto &binds_data = unit_bind_data["binds"];
    for (auto &[bind_key, matrix_json] : binds_data.items()) {
        auto split_pair = spilt_unitkey(bind_key, '|');
        std::string tmpstr1 = split_pair.first;
        std::string tmpstr2 = split_pair.second;
        if (tmpstr1 > tmpstr2) {
            std::swap(tmpstr1, tmpstr2);
        }
        std::string ports_conf_key = tmpstr1 + "|" + tmpstr2;

        auto &unit_data1 = unit_instances_map_[split_pair.first];
        auto &unit_data2 = unit_instances_map_[split_pair.second];

        /* Only support one core. */
        for (auto &arr : matrix_json) {
            matrix.push_back(std::vector<int>());
            for (auto &elm : arr) {
                matrix.back().push_back(elm.get<int>());
            }
        }
        std::vector<std::string> ports_bind;
        for (auto &elm : port_conf_data["bindings"][ports_conf_key]) {
            // std::cout << elm.get<std::string>() << std::endl;
            ports_bind.push_back(elm.get<std::string>());
        }
        bind(unit_data1.nodes, 0, unit_data1.nodes.size(), unit_data2.nodes, 0, unit_data2.nodes.size(),
             matrix, ports_bind);
    }

    /* Bind port. */
}

void UnitBuilder::bind(std::vector<sparta::ResourceTreeNode *> &unitset1, int beg1, int end1,
                       std::vector<sparta::ResourceTreeNode *> &unitset2, int beg2, int end2,
                       const std::vector<std::vector<int>> &matrix,
                       const std::vector<std::string> &ports_bind) {
    for (int i = beg1; i < end1; ++i) {
        for (int j = beg2; j < end2; ++j) {
            if (matrix[i][j] == 0) continue;
            /* Bind ports. */
            for (size_t pos = 0; pos < ports_bind.size(); pos += 2) {
                sparta::bind(unitset1[i]->getChildAs<sparta::Port>("ports." + ports_bind[pos]),
                             unitset2[j]->getChildAs<sparta::Port>("ports." + ports_bind[pos + 1]));
                // std::cout << unitset1[i]->getName() << " " << ports_bind[pos] << " " << unitset2[j]->getName() << " " << ports_bind[pos + 1] << std::endl;
            }
        }
    }
}

}