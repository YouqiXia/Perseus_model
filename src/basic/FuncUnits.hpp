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

    inline FuncMap getFuncMap() {
        FuncMap func_map{
                {"ALU1", {FuncType::ALU, FuncType::MUL, FuncType::DIV, FuncType::BRU, FuncType::CSR}},
                {"ALU2", {FuncType::ALU, FuncType::MUL, FuncType::DIV, FuncType::BRU, FuncType::CSR}},
                {"ALU3", {FuncType::ALU, FuncType::MUL, FuncType::DIV, FuncType::BRU, FuncType::CSR}},
                {"ALU4", {FuncType::ALU, FuncType::MUL, FuncType::DIV, FuncType::BRU, FuncType::CSR}},
                {"LSU" , {FuncType::STU, FuncType::LDU}}
        };
        return func_map;
    }


}