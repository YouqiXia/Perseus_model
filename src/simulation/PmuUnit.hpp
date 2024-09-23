//
// Created by yzhang on 9/22/24.
//

#ifndef PERSEUS_PMUUNIT_HPP
#define PERSEUS_PMUUNIT_HPP

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/simulation/TreeNode.hpp"

namespace TimingModel {

    class PmuUnit : public sparta::Unit {
    public:
        class PmuUnitParam : public sparta::ParameterSet {
        public:
            PmuUnitParam(sparta::TreeNode *n) :
                    sparta::ParameterSet(n) {}

            PARAMETER(bool, turn_on, false, "description")

        };

        static constexpr char name[] = "pmu";

        PmuUnit(sparta::TreeNode *node, const PmuUnitParam *p);

        ~PmuUnit() = default;
    };

}
#endif //PERSEUS_PMUUNIT_HPP
