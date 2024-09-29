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
        std::cout << "-------------------------------------------------------------" << std::endl;
        std::cout << std::setw(32) << std::left << "Performance" << "|" 
                  << std::right << std::setw(12) << "Total" << " |" 
                  << std::setw(12) << "Average" << std::endl;
        std::cout << "-------------------------------------------------------------" << std::endl;
        for (auto& performance_pair: performance_map_) {
            std::cout << std::setw(32) << std::left << performance_pair.first << "|"
                      << std::right << std::setw(12) << performance_pair.second << " |" 
                      << std::setw(12) << (double)performance_pair.second / cycles << std::endl;
        }
        std::cout << "================================================================" << std::endl;
    }

    void PmuUnit::Monitor(std::string instance_name, std::string perf_stat, uint64_t num) {
        if (!pmu_on_) {
            return;
        }

        performance_map_[instance_name + ": " + perf_stat] += num;
    }

    void PmuUnit::Monitor(std::string instance_name, std::string perf_stat, uint64_t num, Mode mode) {
        if (!pmu_on_) {
            return;
        }

        if (mode == Mode::MAX) {
            performance_map_[instance_name + ": " + perf_stat] = 
                std::max<uint64_t>(performance_map_[instance_name + ": " + perf_stat], num);
        }
    }

    void PmuUnit::AllocateHardenParam(std::string instance_name, uint64_t num) {
        if (!pmu_on_) {
            return;
        }
        
        if (harden_param_map_.find(instance_name) == harden_param_map_.end()) {
            harden_param_map_[instance_name] = num;
        }
    }

    uint64_t PmuUnit::GetHardenParam(std::string instance_name) {
        return harden_param_map_.at(instance_name);
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