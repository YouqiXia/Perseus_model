//
// Created by yzhang on 1/14/24.
//

#pragma once

#include <map>
#include <string>

#include "sparta/simulation/ResourceFactory.hpp"

#include "instgen/MavisUnit.hpp"
#include "basic/GlobalParamUnit.hpp"

#include "core/perfect_frontend/PerfectFrontend.hpp"

#include "core/abstract_backend/RenamingStage.hpp"
#include "core/abstract_backend/Rob.hpp"
#include "core/abstract_backend/DispatchStage.hpp"
#include "core/abstract_backend/PhysicalRegfile.hpp"
#include "core/abstract_backend/ReservationStation.hpp"
#include "core/abstract_backend/FlushManager.hpp"

#include "core/func_unit/PerfectFu.hpp"
#include "core/func_unit/WriteBackStage.hpp"

namespace TimingModel {
    class ResourceMapFactory {
    private:
        std::map<std::string, sparta::ResourceFactoryBase*> factories_map;
        std::map<std::string, sparta::Unit> units_map;

    public:
        ResourceMapFactory() {
            // Resource Factory
            // allocators
            RegisterResource_(TimingModel::SelfAllocatorsUnit::name ,
                              new sparta::ResourceFactory<TimingModel::SelfAllocatorsUnit,
                                      TimingModel::SelfAllocatorsUnit::SelfAllocatorParameter>);
            // pmu
            RegisterResource_(TimingModel::PmuUnit::name ,
                              new sparta::ResourceFactory<TimingModel::PmuUnit,
                                      TimingModel::PmuUnit::PmuUnitParam>);

            // mavis
            RegisterResource_(TimingModel::MavisUnit::name ,
                             new MavisFactoy);

            // Global param
            RegisterResource_(TimingModel::GlobalParamUnit::name ,
                              new sparta::ResourceFactory<TimingModel::GlobalParamUnit,
                                      TimingModel::GlobalParamUnit::GlobalParameter>);

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