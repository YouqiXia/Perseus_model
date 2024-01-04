// <OlympiaAllocators.hpp> -*- C++ -*-

#pragma once

/*!
 * \file OlympiaAllocators.hpp
  * \brief Defines a general TreeNode that contains all allocators used
 *        in simulation
 */

#include "sparta/simulation/TreeNode.hpp"
#include "MemAccessInfo.hpp"
#include "Inst.hpp"

namespace TimingModel
{
    /*!
     * \class OlympiaAllocators
     * \brief A TreeNode that is actually a functional resource
     *        containing memory allocators
     */
    class OlympiaAllocators : public sparta::TreeNode
    {
    public:
        static constexpr char name[] = "olympia_allocators";

        OlympiaAllocators(sparta::TreeNode *node) :
            sparta::TreeNode(node, name, "Allocators used in simulation")
        {}

        static OlympiaAllocators * getOlympiaAllocators(sparta::TreeNode *node)
        {
            OlympiaAllocators * allocators = nullptr;
            if(node)
            {
                if(node->hasChild(OlympiaAllocators::name)) {
                    // If this class is converted to a resource, use this line
                    //allocators = node->getChild(OlympiaAllocators::name)->getResourceAs<OlympiaAllocators>();
                    allocators = node->getChildAs<OlympiaAllocators>(OlympiaAllocators::name);
                }
                else {
                    return getOlympiaAllocators(node->getParent());
                }
            }
            return allocators;
        }

        // Allocators used in simulation.  These values can be
        // parameterized in the future by converting this class into a
        // full-blown sparta::Resource and adding a sparta::ParameterSet
        InstAllocator         inst_allocator          {3000, 2500};
        InstArchInfoAllocator inst_arch_info_allocator{3000, 2500};
        MemAccInfoAllocator   mem_acc_info_allocator  {3000, 2500};
    };
}
