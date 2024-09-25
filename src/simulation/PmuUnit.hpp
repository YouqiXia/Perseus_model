//
// Created by yzhang on 9/22/24.
//

#ifndef PERSEUS_PMUUNIT_HPP
#define PERSEUS_PMUUNIT_HPP

#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "sparta/simulation/TreeNode.hpp"

#include "sparta/statistics/Counter.hpp"
#include "sparta/statistics/StatisticDef.hpp"
#include "sparta/statistics/StatisticInstance.hpp"

namespace TimingModel {

    class PmuUnit : public sparta::Unit {
    public:
        class PmuUnitParam : public sparta::ParameterSet {
        public:
            PmuUnitParam(sparta::TreeNode *n) :
                    sparta::ParameterSet(n) {}

            PARAMETER(bool, turn_on, false, "turn on the pmu")
        };

        static constexpr char name[] = "pmu";

        PmuUnit(sparta::TreeNode *node, const PmuUnitParam *p);

        ~PmuUnit();

        bool IsPmuOn() { return pmu_on_; }

        void TurnOn() { pmu_on_ = true; }

        void TurnOff() { pmu_on_ = false; }

        void Monitor(std::string instance_name, std::string perf_stat, uint64_t num);

    private:
        bool pmu_on_;

        std::unordered_map<std::string, uint64_t> performance_map_;
    };

    PmuUnit* getPmuUnit(sparta::TreeNode *node);

}
#endif //PERSEUS_PMUUNIT_HPP
