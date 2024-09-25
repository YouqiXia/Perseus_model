//
// Created by yzhang on 9/22/24.
//

#include "PmuUnit.hpp"

namespace TimingModel {

    PmuUnit::PmuUnit(sparta::TreeNode *node, const PmuUnitParam *p) :
        sparta::Unit(node),
        pmu_on(p->turn_on),
        stat_cycles_(&unit_stat_set_,
                     "cycles",
                     "Total cycle number",
                     &unit_stat_set_,
                     "cycles"),
        cycles_(&stat_cycles_)
    {

    }

    PmuUnit::~PmuUnit() {
        std::cout << "=============================================================" << std::endl;
        for(auto& performance_pair: performance_map_) {
            std::cout << performance_pair.first << ":\t" << performance_pair.second << std::endl;
            std::cout << "Total cycle: " << cycles_.getValue() << std::endl;
            std::cout << "Percentage: " << performance_pair.second / cycles_.getValue() << std::endl;
        }
        
        std::cout << "=============================================================" << std::endl;
    }

    void PmuUnit::Monitor(std::string instance_name, std::string perf_stat, int64_t num) {
        if (!pmu_on) {
            return;
        }
        
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