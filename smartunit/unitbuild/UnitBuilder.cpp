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

static std::vector<std::string> spilt_by_deli(const std::string &unitkey, char deli) {
    std::vector<std::string> result;
    size_t beg = 0;
    size_t pos = unitkey.find(deli, beg);
    while (pos != std::string::npos) {
        result.push_back(unitkey.substr(beg, pos - beg));
        beg = pos + 1;
        pos = unitkey.find(deli, beg);
    }
    result.push_back(unitkey.substr(beg, unitkey.size() - beg));
    return result;
}

static std::pair<int, int> get_location(const std::string &location) {
    int num = std::stoi(location);
    return std::pair<int, int>(num / 100, num % 100);
}

UnitBuilder::UnitBuilder(const std::string &conf_dir) {
    params_.unit_conf_path = conf_dir + "/unit_conf.json";
    params_.unit_bind_path = conf_dir + "/port_conf.json";
    params_.port_conf_path = conf_dir + "/unit_bind.json";
    params_.default_params_path = conf_dir + "/default_params.json";
}

UnitBuilder::UnitBuilder(const UnitBuilder::Params &params) : params_(params) {}

std::vector<sparta::TreeNode *> UnitBuilder::build(sparta::RootTreeNode *root) {
    /* Build tree. */
    /* Add extension into root. */
    for (auto &[extension_name, factory] : TimingModel::UnitRegister::instance().getExtensionFactorys()) {
        root->addExtensionFactory(extension_name, factory);
    }

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

    auto &all_factory = UnitRegister::instance().getAllFactory();
    std::unordered_map<RegisterType, sparta::TreeNode *> module_map;

    for (const auto &[location_key, location_val] : unit_conf_data.items()) {
        int int_location_key = std::stoi(location_key);
        if (final_unit_instances_map_.count(int_location_key) == 0) {
            final_unit_instances_map_.try_emplace(int_location_key);
        }

        auto [cluster_idx, core_idx] = get_location(location_key);
        for (const auto &[unit_key, unit_val] : location_val.items()) {
            size_t create_cnt = unit_val.at(0);
            auto split_pair = spilt_unitkey(unit_key, '.');
            std::string modulename = split_pair.first;
            std::string unitname = split_pair.second;
            auto register_type = str_to_registertype(modulename);

            final_unit_instances_map_[int_location_key].try_emplace(unit_key);

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

            for (size_t i = 0; i < create_cnt; ++i) {
                std::string instance_name = unitname + "_" + std::to_string(cluster_idx)
                                            + "_" + std::to_string(core_idx)
                                            + "_" + std::to_string(i);

                auto node = new sparta::ResourceTreeNode(parent, instance_name, sparta::TreeNode::GROUP_NAME_NONE,
                                                         sparta::TreeNode::GROUP_IDX_NONE,
                                                         instance_name, factory);
                nodes.push_back(node);
                final_unit_instances_map_[int_location_key][unit_key].push_back(node);

                /* Default params setup. */
                if (default_params_data.contains(unit_key)) {
                    auto &key_default_params = default_params_data[unit_key];
                    for (const auto &[param_key, param_value] : key_default_params.items()) {
                        node->getParameterSet()->getParameter(param_key)->setValueFromString(param_value.get<std::string>());
                        // std::cout << "gparam: " << param_key << " " << param_value.get<std::string>() << std::endl;
                    }
                }

                /* Local params setup. */
                if (not unit_val.at(1).empty()) {
                    for (const auto &[param_key, param_value] : unit_val.at(1).items()) {
                        if (not node->getParameterSet()->hasParameter(param_key)) continue;
                        node->getParameterSet()->getParameter(param_key)->setValueFromString(param_value[i].get<std::string>());
                        // std::cout << "lparam: " << param_key << " " << param_value[i].get<std::string>() << std::endl;
                    }
                }
            }
        }
    }

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
    for (auto &[bind_key, matrix_json] : unit_bind_data.items()) {
        auto thekeys = spilt_by_deli(bind_key, '|');
        std::string tmpstr1 = thekeys[0];
        std::string tmpstr2 = thekeys[2];
        bool in_order = true;
        if (tmpstr1 > tmpstr2) {
            std::swap(tmpstr1, tmpstr2);
            in_order = false;
        }
        std::string ports_conf_key = tmpstr1 + "|" + tmpstr2;

        int location1 = std::stoi(thekeys[1]);
        int location2 = std::stoi(thekeys[3]);
        auto &unit_data1 = final_unit_instances_map_[location1][thekeys[0]];
        auto &unit_data2 = final_unit_instances_map_[location2][thekeys[2]];

        /* Only support one core. */
        for (auto &arr : matrix_json) {
            matrix.push_back(std::vector<int>());
            for (auto &elm : arr) {
                matrix.back().push_back(elm.get<int>());
            }
        }
        std::vector<std::string> ports_bind;
        for (auto &elm : port_conf_data[ports_conf_key]) {
            std::cout << elm.get<std::string>() << std::endl;
            ports_bind.push_back(elm.get<std::string>());
        }
        bind(unit_data1, 0, unit_data1.size(), unit_data2, 0, unit_data2.size(),
             matrix, ports_bind, in_order);
    }
}

void UnitBuilder::bind(std::vector<sparta::ResourceTreeNode *> &unitset1, int beg1, int end1,
                       std::vector<sparta::ResourceTreeNode *> &unitset2, int beg2, int end2,
                       const std::vector<std::vector<int>> &matrix,
                       const std::vector<std::string> &ports_bind, bool in_order) {
    for (int i = beg1; i < end1; ++i) {
        for (int j = beg2; j < end2; ++j) {
            if (matrix[i][j] == 0) continue;
            /* Bind ports. */
            for (size_t pos = 0; pos < ports_bind.size(); pos += 2) {
                int idx1 = pos;
                int idx2 = pos + 1;
                if (not in_order) {
                    std::swap(idx1, idx2);
                }
                sparta::bind(unitset1[i]->getChildAs<sparta::Port>("ports." + ports_bind[idx1]),
                             unitset2[j]->getChildAs<sparta::Port>("ports." + ports_bind[idx2]));
                // std::cout << unitset1[i]->getName() << " " << ports_bind[idx1] << " " << unitset2[j]->getName() << " " << ports_bind[idx2] << std::endl;
            }
        }
    }
}

}