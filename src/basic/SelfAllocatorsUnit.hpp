// <OlympiaAllocators.hpp> -*- C++ -*-

#pragma once

/*!
 * \file OlympiaAllocators.hpp
  * \brief Defines a general TreeNode that contains all allocators used
 *        in simulation
 */

#include "sparta/simulation/TreeNode.hpp"
#include "Inst.hpp"
#include "PortInterface.hpp"

namespace TimingModel
{
    /*!
     * \class OlympiaAllocators
     * \brief A TreeNode that is actually a functional resource
     *        containing memory allocators
     */
    class SelfAllocatorsUnit : public sparta::Unit
    {
    public:
        class SelfAllocatorParameter: public sparta::ParameterSet {
        public:
            SelfAllocatorParameter(sparta::TreeNode* n):
                    sparta::ParameterSet(n)
            {}

            PARAMETER(uint64_t, inst_max_num, 3000, "the max number of instructions pointer")
            PARAMETER(uint64_t, inst_arch_info_max_num, 3000, "the max number of instructions arch info pointer")
            PARAMETER(uint64_t, inst_group_max_num, 3000, "the max number of instructions groups pointer")
            PARAMETER(uint64_t, credit_pair_max_num, 3000, "the max number of credit pair pointer")
            PARAMETER(uint64_t, inst_group_pair_max_num, 3000, "the max number of instructions group pair pointer")
        };

        static constexpr char name[] = "self_allocators";

        SelfAllocatorsUnit(sparta::TreeNode *node, const SelfAllocatorParameter* p);

        ~SelfAllocatorsUnit() = default;
        // Allocators used in simulation.  These values can be
        // parameterized in the future by converting this class into a
        // full-blown sparta::Resource and adding a sparta::ParameterSet
        InstAllocator*          inst_allocator;
        InstArchInfoAllocator*  inst_arch_info_allocator;
        InstGroupAllocator*     instgroup_allocator;
        CreditPairAllocator*    credit_pair_allocator;
        InstGroupPairAllocator* inst_group_pair_allocator;
    };

    SelfAllocatorsUnit* getSelfAllocators(sparta::TreeNode *node);
}
