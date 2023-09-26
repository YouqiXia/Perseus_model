#include "Simulator.hh"

namespace Emulator {

    Simulator::Simulator(const YAML::Node &Config) {
        const YAML::Node dram_config = Config["DRAM"];
        dram_ptr_ = std::make_shared<PerfectMemory>(
                "DRAM",
                dram_config["MemoryMap"]["Base"].as<uint64_t>(),
                dram_config["MemoryMap"]["Length"].as<uint64_t>()
        );
    }


}