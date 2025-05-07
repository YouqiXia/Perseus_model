#include "UnitRegister.hpp"

#include "instgen/MavisUnit.hpp"
#include "basic/GlobalParamUnit.hpp"

#include "core/func_unit/PerfectFu.hpp"
#include "core/func_unit/WriteBackStage.hpp"

namespace TimingModel {

int registerExampleUnit(UnitRegister &unitRegister) {
    /* Do your register here. */
    unitRegister.doRegister(RegisterType::Util, TimingModel::SelfAllocatorsUnit::name ,
            new sparta::ResourceFactory<TimingModel::SelfAllocatorsUnit,
                    TimingModel::SelfAllocatorsUnit::SelfAllocatorParameter>);
    // pmu
    unitRegister.doRegister(RegisterType::Util, TimingModel::PmuUnit::name ,
            new sparta::ResourceFactory<TimingModel::PmuUnit,
                    TimingModel::PmuUnit::PmuUnitParam>);

    // mavis
    unitRegister.doRegister(RegisterType::Util, TimingModel::MavisUnit::name ,
            new MavisFactoy);

    // Global param
    unitRegister.doRegister(RegisterType::Util, TimingModel::GlobalParamUnit::name ,
            new sparta::ResourceFactory<TimingModel::GlobalParamUnit,
                    TimingModel::GlobalParamUnit::GlobalParameter>);

    // Function Unit
    unitRegister.doRegister(RegisterType::FU, TimingModel::PerfectFu::name ,
            new sparta::ResourceFactory<TimingModel::PerfectFu,
                    TimingModel::PerfectFu::PerfectFuParameter>);

    unitRegister.doRegister(RegisterType::FU, TimingModel::WriteBackStage::name ,
            new sparta::ResourceFactory<TimingModel::WriteBackStage,
                    TimingModel::WriteBackStage::WriteBackStageParameter>);
    return 0;
}

}