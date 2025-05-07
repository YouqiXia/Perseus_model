#pragma once

#include <map>
#include <string>
#include <memory>

#include <sparta/simulation/ResourceFactory.hpp>
#include <sparta/simulation/Unit.hpp>

namespace TimingModel {

enum class RegisterType {
    FE,
    BE,
    NOC,
    LSU,
    Vector,
    Cache,
    Util,
    FU,
};
std::string registertype_to_str(RegisterType type);
RegisterType str_to_registertype(const std::string &str);

using ElmCreateFunc = std::function<sparta::ResourceFactoryBase*(void)>;
using FactoryMap = std::map<std::string, sparta::ResourceFactoryBase*>;

class UnitRegister {
public:
    /**
     * @brief The execute entry of all unit register 
     * 
     * @return int 0 means successful. Not 0 means failed.
     */
    static int doAllRegister();
    static UnitRegister &instance();

    std::unordered_map<RegisterType, FactoryMap> &getAllFactory();

    /**
     * @brief Register unit factory by using ElmCreateFunc.
     * 
     * @param type 
     * @param resource_name 
     * @param func 
     * @return int 0 means successful. Not 0 means failed.
     */
    int doRegister(RegisterType type, const std::string &resource_name, ElmCreateFunc func);

    /**
     * @brief Register unit factory.
     * 
     * @param type 
     * @param resource_name 
     * @param factory 
     * @return int 
     */
    int doRegister(RegisterType type, const std::string &resource_name, sparta::ResourceFactoryBase* factory);

private:
    UnitRegister();
    ~UnitRegister();
    UnitRegister(const UnitRegister &that) = delete;
    UnitRegister(UnitRegister &&that) = delete;

    bool m_enabled;
    std::unordered_map<RegisterType, FactoryMap> m_all_factories_maps;
};

}