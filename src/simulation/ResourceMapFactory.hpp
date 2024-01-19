//
// Created by yzhang on 1/14/24.
//

#pragma once

#include <map>
#include <string>

#include "sparta/simulation/ResourceFactory.hpp"

#include "olympia/MavisUnit.hpp"

#include "Core/Frontend/PerfectFrontend.hpp"

#include "Core/Backend/RenamingStage.hpp"
#include "Core/Backend/Rob.hpp"
#include "Core/Backend/DispatchStage.hpp"
#include "Core/Backend/PhysicalRegfile.hpp"
#include "Core/Backend/ReservationStation.hpp"

#include "Core/FuncUnit/PerfectAlu.hpp"
#include "Core/FuncUnit/PerfectLsu.hpp"
#include "Core/FuncUnit/LSU/LSUShell.hpp"
#include "Core/FuncUnit/LSU/Agu.hpp"
#include "Core/FuncUnit/LSU/Lsq.hpp"
#include "Core/FuncUnit/WriteBackStage.hpp"
#include "uncore/cache/BaseCache.hpp"
#include "uncore/memory/AbstractMemory.hpp"
#include "uncore/memory/DRAMsim3.hpp"

namespace TimingModel {
    class ResourceMapFactory {
    private:
        std::map<std::string, sparta::ResourceFactoryBase*> factories_map;

    public:
        ResourceMapFactory() {
            // Resource Factory
            // mavis
            RegisterResource(TimingModel::MavisUnit::name ,
                             new MavisFactoy);

            // Frontend
            RegisterResource(TimingModel::PerfectFrontend::name ,
                             new sparta::ResourceFactory<TimingModel::PerfectFrontend,
                                     TimingModel::PerfectFrontend::PerfectFrontendParameter>);
            // Backend
            RegisterResource(TimingModel::RenamingStage::name ,
                             new sparta::ResourceFactory<TimingModel::RenamingStage,
                                     TimingModel::RenamingStage::RenamingParameter>);

            RegisterResource(TimingModel::Rob::name ,
                             new sparta::ResourceFactory<TimingModel::Rob,
                                     TimingModel::Rob::RobParameter>);

            RegisterResource(TimingModel::DispatchStage::name ,
                             new sparta::ResourceFactory<TimingModel::DispatchStage,
                                     TimingModel::DispatchStage::DispatchStageParameter>);

            RegisterResource(TimingModel::PhysicalRegfile::name ,
                             new sparta::ResourceFactory<TimingModel::PhysicalRegfile,
                                     TimingModel::PhysicalRegfile::PhysicalRegfileParameter>);

            RegisterResource(TimingModel::ReservationStation::name ,
                             new sparta::ResourceFactory<TimingModel::ReservationStation,
                                     TimingModel::ReservationStation::ReservationStationParameter>);

            // Function Unit
            RegisterResource(TimingModel::PerfectAlu::name ,
                             new sparta::ResourceFactory<TimingModel::PerfectAlu,
                                     TimingModel::PerfectAlu::PerfectAluParameter>);

            RegisterResource(TimingModel::PerfectLsu::name ,
                             new sparta::ResourceFactory<TimingModel::PerfectLsu,
                                     TimingModel::PerfectLsu::PerfectLsuParameter>);

            RegisterResource(TimingModel::LSUShell::name ,
                             new sparta::ResourceFactory<TimingModel::LSUShell,
                                     TimingModel::LSUShell::LSUShellParameter>);

            RegisterResource(TimingModel::AGU::name ,
                             new sparta::ResourceFactory<TimingModel::AGU,
                                     TimingModel::AGU::AGUParameter>);

            RegisterResource(TimingModel::LSQ::name ,
                             new sparta::ResourceFactory<TimingModel::LSQ,
                                     TimingModel::LSQ::LSQParameter>);

            RegisterResource(TimingModel::WriteBackStage::name ,
                             new sparta::ResourceFactory<TimingModel::WriteBackStage,
                                     TimingModel::WriteBackStage::WriteBackStageParameter>);
            RegisterResource("l1d_cache" ,
                             new sparta::ResourceFactory<TimingModel::BaseCache,
                                     TimingModel::BaseCache::BaseCacheParameterSet>);
            RegisterResource("l2_cache",
                             new sparta::ResourceFactory<TimingModel::BaseCache,
                                     TimingModel::BaseCache::BaseCacheParameterSet>);
            RegisterResource(TimingModel::AbstractMemroy::name,
                             new sparta::ResourceFactory<TimingModel::AbstractMemroy,
                                     TimingModel::AbstractMemroy::AbstractMemroyParameterSet>);
            RegisterResource(TimingModel::DRAMsim3::name,
                            new sparta::ResourceFactory<TimingModel::DRAMsim3,
                                    TimingModel::DRAMsim3::DRAMsim3ParameterSet>);
        };

        ~ResourceMapFactory() {}

        void RegisterResource(std::string resource_name, sparta::ResourceFactoryBase* factory_ptr) {
            assert(factories_map.find(resource_name) == factories_map.end());
            factories_map[resource_name] = factory_ptr;
        }

        sparta::ResourceFactoryBase* operator[](std::string resource_name) {
            assert(factories_map.find(resource_name) != factories_map.end());
            return factories_map[resource_name];
        }
    };
}