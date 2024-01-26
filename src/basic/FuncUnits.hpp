//
// Created by yzhang on 1/4/24.
//

#pragma once

#include <array>
#include <string>
#include <map>
#include <set>

#include "Inst.hpp"

namespace TimingModel {

    inline FuncType stringToFuncType(std::string & str){
        FuncType type = FuncType::NO_TYPE;
        if(str.compare("FuncType::ALU") == 0)       return FuncType::ALU;
        else if(str.compare("FuncType::MUL") == 0)  return FuncType::MUL;
        else if(str.compare("FuncType::DIV") == 0)  return FuncType::DIV;
        else if(str.compare("FuncType::BRU") == 0)  return FuncType::BRU;
        else if(str.compare("FuncType::CSR") == 0)  return FuncType::CSR;
        else if(str.compare("FuncType::LDU") == 0)  return FuncType::LDU;
        else if(str.compare("FuncType::STU") == 0)  return FuncType::STU;
        else if(str.compare("FuncType::FPU") == 0)  return FuncType::FPU;
        else                                        return FuncType::NO_TYPE;
    }

    inline std::string funcTypeToString(FuncType type){
        switch(type){
            case FuncType::ALU:     return "FuncType::ALU";
            case FuncType::MUL:     return "FuncType::MUL";
            case FuncType::DIV:     return "FuncType::DIV";
            case FuncType::BRU:     return "FuncType::BRU";
            case FuncType::CSR:     return "FuncType::CSR";
            case FuncType::LDU:     return "FuncType::LDU";
            case FuncType::STU:     return "FuncType::STU";
            case FuncType::FPU:     return "FuncType::FPU";
            default:
                break;
        }
        return nullptr;
    }

    using RsType = std::string;
    using FuncUnitType = std::string;
    using DispatchMap = std::map<RsType, std::set<FuncType>>;
    using FuMap = std::vector<std::string>;

    DispatchMap& getDispatchMap();

    FuMap& getFuMap();

    void setDispatchMap(sparta::TreeNode * rootnode);

    void setFuMap(sparta::TreeNode * rootnode);

    void printFuMap();

    void setFuCfgFromExtensions(sparta::TreeNode * rootnode);
}