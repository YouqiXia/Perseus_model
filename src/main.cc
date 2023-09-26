#include <string>
#include <iostream>
#include "cmdline/cmdline.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "trace/Logging.hh"
#include "yaml-cpp/yaml.h"

void InitFlags(const cmdline::parser &);
std::vector<std::string> split(std::string, std::string);

int main(int argc, char *argv[]) {
    cmdline::parser cmdParser;
    cmdParser.add<std::string>("config", 'c', "Simulator Config Path", true);
    cmdParser.add<std::string>("elf", 'e', "elf Path", true);
    cmdParser.add<std::string>("log", 'l', "Log Path", false);
    cmdParser.add<std::string>("debugFlag", 'd', "Open Debug Log", false);
    cmdParser.add<uint64_t>("MaxCycle", 'm', "Max Tick Cycle", false, -1);
    cmdParser.add("debugHelp", 'h', "Print debug Flags description");
    cmdParser.parse_check(argc, argv);

    // Trace Log Config
    InitFlags(cmdParser);

    const YAML::Node config = YAML::LoadFile(cmdParser.get<std::string>("config"));

}

void InitFlags(const cmdline::parser &cmdParser) {
    if (cmdParser.exist("debugHelp")) {
        std::cout << "[---------- Debug flags descriptions ----------]" << std::endl;
        for (auto pair: Emulator::debugFlags) {
            std::cout << fmt::format("[ {:<17} {:^12} {:>17} ]", pair.first, "|", pair.second) << std::endl;
        }
        std::cout << "\n";
        std::cout << "[---------- Debug Object descriptions ----------]" << std::endl;
        for (auto pair: Emulator::registedObject) {
            std::cout << fmt::format("[ {:<17} {:^12} {:>17} ]", pair.first, "|", pair.second) << std::endl;
        }
        exit(0);
    }

    /* Trace Log Config */
    const bool    console  = !cmdParser.exist("log");
    const bool    debug    = cmdParser.exist("debugFlag");
    const std::string  logPath  = cmdParser.get<std::string>("log");
    Emulator::Trace::initLogger(debug,console,logPath);

    /* Init Debug Flags */
    if(debug){
        const std::vector<std::string> flags = split(cmdParser.get<std::string>("debugFlag"),",");
        bool                 openall = false;
        for(const auto& flag : flags){
            if(flag == std::string("ALL")){
                openall = true;
            }
        }
        if(openall){
            for(const auto& flag : Emulator::debugFlags){
                Emulator::setFlagEnable(flag.first);
            }
            for(const auto& object : Emulator::registedObject){
                Emulator::setObjectEnable(object.first);
            }
        } else {
            for(auto flag : flags){
                if(Emulator::debugFlags.count(flag)){
                    Emulator::setFlagEnable(flag);
                }else if(Emulator::registedObject.count(flag)){
                    Emulator::setObjectEnable(flag);
                }else{
                    SPDLOG_ERROR("Known Flag : {}",flag);
                    exit(-1);
                }
            }
        }
    }
}

std::vector<std::string> split(std::string str, std::string pattern)
{
    std::string::size_type pos;
    std::vector<std::string> result;
    str += pattern;
    int size = str.size();
    for (int i = 0; i < size; i++)
    {
        pos = str.find(pattern, i);
        if (pos < size)
        {
            std::string s = str.substr(i, pos - i);
            result.push_back(s);
            i = pos + pattern.size() - 1;
        }
    }
    return result;
}