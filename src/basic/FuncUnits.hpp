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

    FuncMap& getFuncMap();
    
    FuncCreditMap& getFuncCredit();

    void setFuMap(sparta::TreeNode * rootnode);

    void printFuMap();

    void setFuCreditMap(sparta::TreeNode * rootnode);

    void printCreditMap();

    void setFuCfgFromExtensions(sparta::TreeNode * rootnode);
}