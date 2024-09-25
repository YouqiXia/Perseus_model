//
// Created by yzhang on 9/22/24.
//

#include "PmuUnit.hpp"

namespace TimingModel {

    PmuUnit::PmuUnit(sparta::TreeNode *node, const PmuUnitParam *p) :
        sparta::Unit(node),
        pmu_on_(p->turn_on)
    {}

    PmuUnit::~PmuUnit() {
        std::cout << "==============================PMU===============================" << std::endl;
        auto cycles = getClock()->currentCycle();
        std::cout << "Total cycle: " << cycles << std::endl;
        for(auto& performance_pair: performance_map_) {
            std::cout << performance_pair.first <<  " total: " << performance_pair.second << ", Percentage: "
            << performance_pair.second / cycles << std::endl;
        }
        std::cout << "================================================================" << std::endl;
    }

    void PmuUnit::Monitor(std::string instance_name, std::string perf_stat, uint64_t num) {
        performance_map_[instance_name + " " + perf_stat] += num;
    }

    PmuUnit* getPmuUnit(sparta::TreeNode *node){
        
        PmuUnit * pmu = nullptr;
        if(node)
        {
            if(node->hasChild(PmuUnit::name)) {
                pmu = node->getChild(PmuUnit::name)->getResourceAs<PmuUnit>();
            }
            else {
                return getPmuUnit(node->getParent());
            }
        }
        return pmu;
    }

}