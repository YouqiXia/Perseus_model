#ifndef MODEL_SIMULATOR_HH
#define MODEL_SIMULATOR_HH

#include <memory>

#include "basicunit/Register.hh"
#include "yaml-cpp/yaml.h"
#include "trace/Logging.hh"
#include "component/PerfectMemory/PerfectMemory.hh"

namespace Emulator {

    class Simulator {
    public:
        Simulator(const YAML::Node& Config);

        ~Simulator();

    private:
        std::shared_ptr<PerfectMemory> dram_ptr_;
    };
}

#endif //MODEL_SIMULATOR_HH
