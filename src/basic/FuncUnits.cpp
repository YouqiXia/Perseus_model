#include "FuncUnits.hpp"
#include "simulation/ExtensionsCfg.hpp"

namespace TimingModel {
    FuncMap func_map{
            {"LSU" , {FuncType::STU, FuncType::LDU}},
            {"ALU1", {FuncType::ALU, FuncType::MUL, FuncType::DIV, FuncType::BRU, FuncType::CSR}},
            {"ALU2", {FuncType::ALU, FuncType::MUL, FuncType::DIV, FuncType::BRU, FuncType::CSR}},
            {"ALU3", {FuncType::ALU, FuncType::MUL, FuncType::DIV, FuncType::BRU, FuncType::CSR}},
            {"ALU4", {FuncType::ALU, FuncType::MUL, FuncType::DIV, FuncType::BRU, FuncType::CSR}}
    };

    FuncCreditMap func_credit_map{
            {"LSU" , 10},
            {"ALU1", 2},
            {"ALU2", 2},
            {"ALU3", 2},
            {"ALU4", 2}
    };


    FuncMap& getFuncMap() {
        return func_map;
    }

    FuncCreditMap& getFuncCredit() {
        return func_credit_map;
    }
    void setFuMap(sparta::TreeNode * rootnode){
        auto fu_map = TimingModel::getFuMap(rootnode);
        uint32_t fu_cnt = fu_map.size();
        if(fu_cnt > 0){
            func_map.clear();
        }else{
            return;
        }

        for(auto i=0u; i<fu_cnt; i++){
            uint32_t type_cnt = fu_map[i].size()-1;
            std::set<FuncType> fu_types;
            for(auto t=0u; t<type_cnt; t++){
                FuncType type = stringToFuncType(fu_map[i][t+1]);
                fu_types.insert(type);
            }
            func_map[fu_map[i][0]] = fu_types;
        }
    }
    void printFuMap(){
        std::cout << "fu map:" << std::endl;
        for(std::map<FuncUnitType, std::set<FuncType>>::iterator it=func_map.begin(); it!=func_map.end(); ++it){
            std::cout << it->first << ": ";
            for (std::set<FuncType>::iterator its =it->second.begin(); its!=it->second.end(); ++its)
                std::cout << funcTypeToString(*its) << ", ";
            std::cout << std::endl;
        }
    }
    void setFuCreditMap(sparta::TreeNode * rootnode){
        auto fu_credit_map = TimingModel::getFuCreditMap(rootnode);
        uint32_t fu_credit_cnt = fu_credit_map.size();
        if(fu_credit_cnt > 0){
            func_credit_map.clear();
        }else{
            return;
        }

        for(auto i=0u; i<fu_credit_cnt; i++){
            func_credit_map[fu_credit_map[i][0]] = std::stoull(fu_credit_map[i][1]);
        }
    }
    void printCreditMap(){
        std::cout << "fu credit map:" << std::endl;
        for(std::map<FuncUnitType, Credit>::iterator it=func_credit_map.begin(); it!=func_credit_map.end(); ++it){
            std::cout << it->first << ": " << it->second << std::endl;
        }
    }
    void setFuCfgFromExtensions(sparta::TreeNode * rootnode){
        setFuMap(rootnode);
        setFuCreditMap(rootnode);
    }
}

