#include "UnitRegister.hpp"

namespace TimingModel {

std::string registertype_to_str(RegisterType type) {
    static const std::unordered_map<TimingModel::RegisterType, std::string> s_type2str = {
        {TimingModel::RegisterType::FE, "FE"},
        {TimingModel::RegisterType::BE, "BE"},
        {TimingModel::RegisterType::NOC, "NOC"},
        {TimingModel::RegisterType::LSU, "LSU"},
        {TimingModel::RegisterType::Vector, "Vector"},
        {TimingModel::RegisterType::Cache, "Cache"},
        {TimingModel::RegisterType::Util, "Util"},
        {TimingModel::RegisterType::FU, "FU"},
    };

    return s_type2str.at(type);
}

RegisterType str_to_registertype(const std::string &str) {
    static const std::unordered_map<std::string, TimingModel::RegisterType> s_str2type = {
        {"FE", TimingModel::RegisterType::FE},
        {"BE", TimingModel::RegisterType::BE},
        {"NOC", TimingModel::RegisterType::NOC},
        {"LSU", TimingModel::RegisterType::LSU},
        {"Vector", TimingModel::RegisterType::Vector},
        {"Cache", TimingModel::RegisterType::Cache},
        {"Util", TimingModel::RegisterType::Util},
        {"FU", TimingModel::RegisterType::FU},
    };

    return s_str2type.at(str);
}

UnitRegister &UnitRegister::instance() {
    static UnitRegister the_instance;
    return the_instance;
}

std::unordered_map<RegisterType, FactoryMap> &UnitRegister::getAllFactory() {
    return m_all_factories_maps;
}

int UnitRegister::doRegister(RegisterType type, const std::string &resource_name, ElmCreateFunc func) {
    if (m_enabled) {
        sparta::ResourceFactoryBase *factory = func();
        return doRegister(type, resource_name, factory);
    } else {
        return -1;
    }
}

int UnitRegister::doRegister(RegisterType type, const std::string &resource_name, sparta::ResourceFactoryBase* factory) {
    if (m_all_factories_maps.count(type) == 0) {
        m_all_factories_maps.emplace(type, FactoryMap());
    }
    auto &factories_map = m_all_factories_maps.at(type);
    if (factories_map.find(resource_name) == factories_map.end()) {
        factories_map[resource_name] = factory;
        return 0;
    } else {
        std::cerr << resource_name << " has been registered in " << (int)type << std::endl;
        return -1;
    }
}

int UnitRegister::doRegisterExtension(const std::string &extension_name, std::function<sparta::TreeNode::ExtensionsBase*()> factory) {
    m_extension_factorys[extension_name] = factory;
    return 0;
}

ExtensionFactorys &UnitRegister::getExtensionFactorys() {
    return m_extension_factorys;
}

UnitRegister::UnitRegister() : m_enabled(true) {}

UnitRegister::~UnitRegister() {
    for (auto &[type, factories_map] : m_all_factories_maps) {
        for (auto &[name, factory_ptr] : factories_map) {
            delete factory_ptr;
        }
    }
}

}