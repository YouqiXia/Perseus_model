//
// Created by yzhang on 1/14/24.
//

#pragma once

#include <map>
#include <string>

#include "sparta/simulation/ResourceFactory.hpp"

#include "olympia/MavisUnit.hpp"
#include "basic/GlobalParam.hpp"

#include "core/perfect_frontend/PerfectFrontend.hpp"

#include "core/abstract_backend/RenamingStage.hpp"
#include "core/abstract_backend/Rob.hpp"
#include "core/abstract_backend/DispatchStage.hpp"
#include "core/abstract_backend/PhysicalRegfile.hpp"
#include "core/abstract_backend/ReservationStation.hpp"
#include "core/abstract_backend/FlushManager.hpp"

#include "core/func_unit/PerfectFu.hpp"
#include "core/func_unit/WriteBackStage.hpp"

#include "uncore/cache/BaseCache.hpp"
#include "uncore/memory/AbstractMemory.hpp"
#include "uncore/memory/DRAMsim3.hpp"


namespace TimingModel {
    class ResourceMapFactory {
    private:
        std::map<std::string, sparta::ResourceFactoryBase*> factories_map;
        std::map<std::string, sparta::Unit> units_map;

    public:
        ResourceMapFactory() {
            // Resource Factory
            // mavis
            RegisterResource_(TimingModel::MavisUnit::name ,
                             new MavisFactoy);

            // Global param
            RegisterResource_(TimingModel::GlobalParam::name ,
                              new sparta::ResourceFactory<TimingModel::GlobalParam,
                                      TimingModel::GlobalParam::GlobalParameter>);

            // Frontend
            RegisterResource_(TimingModel::PerfectFrontend::name ,
                             new sparta::ResourceFactory<TimingModel::PerfectFrontend,
                                     TimingModel::PerfectFrontend::PerfectFrontendParameter>);
            // abstract_backend
            RegisterResource_(TimingModel::RenamingStage::name ,
                             new sparta::ResourceFactory<TimingModel::RenamingStage,
                                     TimingModel::RenamingStage::RenamingParameter>);

            RegisterResource_(TimingModel::Rob::name ,
                             new sparta::ResourceFactory<TimingModel::Rob,
                                     TimingModel::Rob::RobParameter>);

            RegisterResource_(TimingModel::DispatchStage::name ,
                             new sparta::ResourceFactory<TimingModel::DispatchStage,
                                     TimingModel::DispatchStage::DispatchStageParameter>);

            RegisterResource_(TimingModel::PhysicalRegfile::name ,
                             new sparta::ResourceFactory<TimingModel::PhysicalRegfile,
                                     TimingModel::PhysicalRegfile::PhysicalRegfileParameter>);

            RegisterResource_(TimingModel::ReservationStation::name ,
                             new sparta::ResourceFactory<TimingModel::ReservationStation,
                                     TimingModel::ReservationStation::ReservationStationParameter>);

            RegisterResource_(TimingModel::FlushManager::name ,
                             new sparta::ResourceFactory<TimingModel::FlushManager,
                                     TimingModel::FlushManager::FlushManagerParameter>);


            // Function Unit
            RegisterResource_(TimingModel::PerfectFu::name ,
                              new sparta::ResourceFactory<TimingModel::PerfectFu,
                                      TimingModel::PerfectFu::PerfectFuParameter>);

//            RegisterResource_(TimingModel::LSUShell::name ,
//                             new sparta::ResourceFactory<TimingModel::LSUShell,
//                                     TimingModel::LSUShell::LSUShellParameter>);

//            RegisterResource_(TimingModel::AGU::name ,
//                             new sparta::ResourceFactory<TimingModel::AGU,
//                                     TimingModel::AGU::AGUParameter>);

//            RegisterResource_(TimingModel::LSQ::name ,
//                             new sparta::ResourceFactory<TimingModel::LSQ,
//                                     TimingModel::LSQ::LSQParameter>);

            RegisterResource_(TimingModel::WriteBackStage::name ,
                             new sparta::ResourceFactory<TimingModel::WriteBackStage,
                                     TimingModel::WriteBackStage::WriteBackStageParameter>);
//            RegisterResource_("l1d_cache" ,
//                             new sparta::ResourceFactory<TimingModel::BaseCache,
//                                     TimingModel::BaseCache::BaseCacheParameterSet>);
//            RegisterResource_("l2_cache",
//                             new sparta::ResourceFactory<TimingModel::BaseCache,
//                                     TimingModel::BaseCache::BaseCacheParameterSet>);
//            RegisterResource_(TimingModel::AbstractMemroy::name,
//                             new sparta::ResourceFactory<TimingModel::AbstractMemroy,
//                                     TimingModel::AbstractMemroy::AbstractMemroyParameterSet>);
//            RegisterResource_(TimingModel::DRAMsim3::name,
//                            new sparta::ResourceFactory<TimingModel::DRAMsim3,
//                                    TimingModel::DRAMsim3::DRAMsim3ParameterSet>);
        };

        ~ResourceMapFactory() {}

        sparta::ResourceFactoryBase* operator[](std::string resource_name) const {
            assert(factories_map.find(resource_name) != factories_map.end());
            return factories_map.at(resource_name);
        }

        std::map<std::string, sparta::ResourceFactoryBase*> GetResourceFactory() const {
            return factories_map;
        }
        
    private:
        void RegisterResource_(std::string resource_name, sparta::ResourceFactoryBase* factory_ptr) {
            assert(factories_map.find(resource_name) == factories_map.end());
            factories_map[resource_name] = factory_ptr;
        }
    };
}