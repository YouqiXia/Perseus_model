//
// Created by yzhang on 9/22/24.
//
#include "SelfAllocatorsUnit.hpp"

namespace TimingModel {

    std::unique_ptr<InstAllocator>          inst_allocator;
    std::unique_ptr<InstArchInfoAllocator>  inst_arch_info_allocator;
    std::unique_ptr<InstGroupAllocator>     instgroup_allocator;
    std::unique_ptr<CreditPairAllocator>    credit_pair_allocator;
    std::unique_ptr<InstGroupPairAllocator> inst_group_pair_allocator;

    SelfAllocatorsUnit::SelfAllocatorsUnit(sparta::TreeNode *node, const SelfAllocatorParameter* p) :
        sparta::Unit(node)
    {
        TimingModel::inst_allocator.reset(new InstAllocator{p->inst_max_num, p->inst_max_num/5*4});
        TimingModel::inst_arch_info_allocator.reset(new InstArchInfoAllocator{p->inst_arch_info_max_num, p->inst_arch_info_max_num/5*4});
        TimingModel::instgroup_allocator.reset(new InstGroupAllocator{p->inst_group_max_num, p->inst_group_max_num/5*4});
        TimingModel::credit_pair_allocator.reset(new CreditPairAllocator{p->credit_pair_max_num, p->credit_pair_max_num/5*4});
        TimingModel::inst_group_pair_allocator.reset(new InstGroupPairAllocator{p->inst_group_pair_max_num, p->inst_group_pair_max_num/5*4});
        inst_allocator = TimingModel::inst_allocator.get();
        inst_arch_info_allocator = TimingModel::inst_arch_info_allocator.get();
        instgroup_allocator = TimingModel::instgroup_allocator.get();
        credit_pair_allocator = TimingModel::credit_pair_allocator.get();
        inst_group_pair_allocator = TimingModel::inst_group_pair_allocator.get();
    }

    SelfAllocatorsUnit* getSelfAllocators(sparta::TreeNode *node)  {

        SelfAllocatorsUnit * allocators = nullptr;
        if(node)
        {
            if(node->hasChild(SelfAllocatorsUnit::name)) {
                allocators = node->getChild(SelfAllocatorsUnit::name)->getResourceAs<SelfAllocatorsUnit>();
            }
            else {
                return getSelfAllocators(node->getParent());
            }
        }
        return allocators;
    }

}