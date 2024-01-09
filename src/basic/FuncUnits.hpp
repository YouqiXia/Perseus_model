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
                {"LSU" , {FuncType::STU, FuncType::LDU}},
                {"ALU1", {FuncType::ALU, FuncType::MUL, FuncType::DIV, FuncType::BRU, FuncType::CSR}},
                {"ALU2", {FuncType::ALU, FuncType::MUL, FuncType::DIV, FuncType::BRU, FuncType::CSR}},
                {"ALU3", {FuncType::ALU, FuncType::MUL, FuncType::DIV, FuncType::BRU, FuncType::CSR}},
                {"ALU4", {FuncType::ALU, FuncType::MUL, FuncType::DIV, FuncType::BRU, FuncType::CSR}}
        };
        return func_map;
    }

    inline FuncCreditMap getFuncCredit() {
        FuncCreditMap func_credit_map{
                {"LSU" , 10},
                {"ALU1", 2},
                {"ALU2", 2},
                {"ALU3", 2},
                {"ALU4", 2}
        };
        return func_credit_map;
    }


}