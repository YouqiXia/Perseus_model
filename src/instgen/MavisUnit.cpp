// <MavisUnit.cpp> -*- C++ -*-

//!
//! \file MavisUnit.cpp
//! \brief A functiona unit of Mavis, placed in the Sparta Tree for any unit to grab/use
//!

#include "mavis/Mavis.h"
#include "MavisUnit.hpp"

#include "basic/SelfAllocatorsUnit.hpp"

namespace TimingModel
{

    std::vector<std::string> getISAFiles(sparta::TreeNode *n, const std::string & isa_file_path,
                                         const std::string& pseudo_file_path)
    {
        std::vector<std::string> isa_files = {isa_file_path + "/isa_rv64g.json",
                                              isa_file_path + "/isa_rv64zba.json",
                                              isa_file_path + "/isa_rv64zbb.json",
                                              isa_file_path + "/isa_rv64zbs.json",
                                              isa_file_path + "/isa_rv64c.json",
                                              isa_file_path + "/isa_rv64cf.json",
                                              isa_file_path + "/isa_rv64cd.json"};
        return isa_files;
    }

    std::vector<std::string> getUArchFiles(sparta::TreeNode *n, const std::string& uarch_overrides_json,
                                           const std::string & uarch_file_path, const std::string& pseudo_file_path)
    {
        std::vector<std::string> uarch_files = {uarch_file_path + "/olympia_uarch_rv64g.json",
                                                uarch_file_path + "/olympia_uarch_rv64c.json",
                                                uarch_file_path + "/olympia_uarch_rv64b.json"};

        if(false == std::string(uarch_overrides_json).empty()) {
            uarch_files.emplace_back(uarch_overrides_json);
        }

        return uarch_files;
    }

    MavisType::AnnotationOverrides getUArchAnnotationOverrides(const std::vector<std::string>& uarch_overrides)
    {
        MavisType::AnnotationOverrides annotations;

        const std::vector<std::string> vals = uarch_overrides;
        for(auto overde : vals)
        {
            sparta_assert(overde.find(',') != std::string::npos,  "Malformed uarch override: " << overde);
            const std::string mnemonic  = overde.substr(0, overde.find(','));
            const std::string attribute_pair = overde.substr(overde.find(',')+1);
            sparta_assert(!mnemonic.empty() and !attribute_pair.empty(), "Malformed uarch override: " << overde);
            annotations.emplace_back(std::make_pair(mnemonic, attribute_pair));
        }

        return annotations;
    }


    /**
     * \brief Construct a new Mavis unit
     * \param n Tree node parent for this unit
     * \param p Unit parameters
     */
    MavisUnit::MavisUnit(sparta::TreeNode *n, const MavisParameters* p) :
        sparta::Unit(n),
        pseudo_file_path_(std::string(p->pseudo_file_path).empty() ? p->uarch_file_path : p->pseudo_file_path),
        node_(n),
        isa_file_path_(p->isa_file_path),
        uarch_file_path_(p->uarch_file_path),
        uarch_overrides_json_(p->uarch_overrides_json),
        uarch_overrides_(p->uarch_overrides)
    {
        sparta::StartupEvent(n, CREATE_SPARTA_HANDLER(MavisUnit, Startup_));
    }

    /**
     * \brief Destruct a mavis unit
     */
    MavisUnit::~MavisUnit() {}

    void MavisUnit::Startup_() {
        MavisType * mavis_type_ptr = new MavisType(getISAFiles(node_, isa_file_path_, pseudo_file_path_),
                                      getUArchFiles(node_, uarch_overrides_json_, uarch_file_path_, pseudo_file_path_),
                                      mavis_uid_list_, getUArchAnnotationOverrides(uarch_overrides_),
                                      InstPtrAllocator<InstAllocator>
                                              (*sparta::notNull(getSelfAllocators(node_))->inst_allocator),
                                      InstPtrAllocator<InstArchInfoAllocator>
                                              (*sparta::notNull(getSelfAllocators(node_))->inst_arch_info_allocator));
        mavis_facade_.reset(mavis_type_ptr);
    }

    /**
     * \brief Sparta-visible global function to find a mavis node and provide the mavis facade
     * \param node Tree node to start the search (recurses up the tree from here until a mavis unit is found)
     * \return Pointer to Mavis facade object
     */
    MavisUnit* getMavis(sparta::TreeNode *node)
    {
        MavisUnit * mavis_unit = nullptr;
        if (node)
        {
            if (node->hasChild(MavisUnit::name)) {
                mavis_unit = node->getChild(MavisUnit::name)->getResourceAs<MavisUnit>();
            }
            else {
                return getMavis(node->getParent());
            }
        }
        sparta_assert(mavis_unit != nullptr, "Mavis unit was not found");
        // cppcheck-suppress nullPointer
        return mavis_unit;
    }

} // namespace olympia
