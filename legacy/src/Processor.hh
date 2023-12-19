#ifndef PROCESSOR_HH_
#define PROCESSOR_HH_

#include <unordered_map>
#include <vector>
#include "yaml-cpp/yaml.h"
#include "basicunit/Register.hh"
#include "basicunit/Stage.hh"

namespace Emulator {

    class ComponentBuilder {
    public:
        ComponentBuilder();
        ~ComponentBuilder();
        std::unordered_map<std::string, std::shared_ptr<Register>> GetComponentMap();
        /* Method to build component */
        /* Common */
        void BuildCSR(const YAML::Node&);
        /* Fetch1 */
        void BuildFetch1Pipe(const YAML::Node&);
        /* Fetch2 */
        void BuildInstBuffer(const YAML::Node&);
        /* Decode */
        void BuildDecodePipe(const YAML::Node&);

    private:
        std::unordered_map<std::string, std::shared_ptr<Register>> component_map_;
    };

    class ComponentDirector {};

    class StageBuilder{
    public:
        StageBuilder();
        ~StageBuilder();
        std::map<std::string, std::shared_ptr<Stage>> GetStageMap();
        /* Method to build stage */
        void BuildFetch1();
        void BuildFetch2();
    private:
        std::map<std::string, std::shared_ptr<Stage>> stage_map_;
    };

    class StageDirector {};

    class Processor {};

}

#endif