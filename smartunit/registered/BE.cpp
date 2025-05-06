#include "BE.hpp"

#include "core/abstract_backend/RenamingStage.hpp"
#include "core/abstract_backend/Rob.hpp"
#include "core/abstract_backend/DispatchStage.hpp"
#include "core/abstract_backend/PhysicalRegfileUnit.hpp"
#include "core/abstract_backend/SchedulerUnit.hpp"
#include "core/abstract_backend/FlushManager.hpp"
#include "core/abstract_backend/BusyTableUnit.hpp"
#include "core/abstract_backend/SpecBusyTableUnit.hpp"
#include "core/abstract_backend/StagingBufferUnit.hpp"

namespace TimingModel {

int registerBE(UnitRegister &unitRegister) {
    unitRegister.doRegister(RegisterType::BE, TimingModel::RenamingStage::name ,
        new sparta::ResourceFactory<TimingModel::RenamingStage,
                TimingModel::RenamingStage::RenamingParameter>);

    unitRegister.doRegister(RegisterType::BE, TimingModel::BusyTableUnit::name ,
            new sparta::ResourceFactory<TimingModel::BusyTableUnit,
                    TimingModel::BusyTableUnit::BusyTableParameter>);

    unitRegister.doRegister(RegisterType::BE, TimingModel::SpecBusyTableUnit::name ,
            new sparta::ResourceFactory<TimingModel::SpecBusyTableUnit,
                    TimingModel::SpecBusyTableUnit::SpecBusyTableParameter>);

    unitRegister.doRegister(RegisterType::BE, TimingModel::StagingBufferUnit::name ,
            new sparta::ResourceFactory<TimingModel::StagingBufferUnit,
                    TimingModel::StagingBufferUnit::StagingBufferParameter>);

    unitRegister.doRegister(RegisterType::BE, TimingModel::Rob::name ,
            new sparta::ResourceFactory<TimingModel::Rob,
                    TimingModel::Rob::RobParameter>);

    unitRegister.doRegister(RegisterType::BE, TimingModel::DispatchStage::name ,
            new sparta::ResourceFactory<TimingModel::DispatchStage,
                    TimingModel::DispatchStage::DispatchStageParameter>);

    unitRegister.doRegister(RegisterType::BE, TimingModel::PhysicalRegfileUnit::name ,
            new sparta::ResourceFactory<TimingModel::PhysicalRegfileUnit,
                    TimingModel::PhysicalRegfileUnit::PhysicalRegfileParameter>);

    unitRegister.doRegister(RegisterType::BE, TimingModel::SchedulerUnit::name ,
            new sparta::ResourceFactory<TimingModel::SchedulerUnit,
                    TimingModel::SchedulerUnit::ReservationStationParameter>);

    unitRegister.doRegister(RegisterType::BE, TimingModel::FlushManager::name ,
            new sparta::ResourceFactory<TimingModel::FlushManager,
                    TimingModel::FlushManager::FlushManagerParameter>);

    return 0;
}

}