#include "Logging.hh"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/async.h"

namespace Emulator
{
    
namespace Trace
{


void initLogger(const bool debug, const bool console, const std::string filePath)
{
    try{
        if(console) {
            auto sink   = std::make_shared<spdlog::sinks::stdout_color_sink_mt>(spdlog::color_mode::automatic);
            static auto TraceLogger = std::make_shared<spdlog::logger>("Model", sink);
            TraceLogger->set_level(debug ? spdlog::level::trace : spdlog::level::err);
            TraceLogger->set_pattern("%^[%l]%$ -> %v");
            TraceLogger->info("Successfully Init Logger(Console)");
            spdlog::register_logger(TraceLogger);
            // TraceLogger = spdlog::stdout_color_mt("Model",spdlog::color_mode::automatic);
        } else {
            auto sink   = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filePath);
            static auto TraceLogger = std::make_shared<spdlog::logger>("Model", sink);
            TraceLogger->set_level(debug ? spdlog::level::trace : spdlog::level::err);
            TraceLogger->set_pattern("%^[%l]%$ -> %v");
            TraceLogger->info("Successfully Init Logger(File : {})",filePath);
            spdlog::register_logger(TraceLogger);
        }
    }
    catch(const spdlog::spdlog_ex& ex){
        std::cout << "Log initialization failed: " << ex.what() << std::endl;
        exit(-1);
    }
}


} // namespace Trace

} // namespace Emulator
