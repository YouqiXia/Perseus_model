#include "FE.hpp"

#include "core/perfect_frontend/PerfectFrontend.hpp"

namespace TimingModel {

int registerPerfectFE(UnitRegister &unitRegister) {
    return unitRegister.doRegister(RegisterType::FE, TimingModel::PerfectFrontend::name, []{
        return new sparta::ResourceFactory<TimingModel::PerfectFrontend, TimingModel::PerfectFrontend::PerfectFrontendParameter>;
    });
}

}