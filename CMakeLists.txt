cmake_minimum_required (VERSION 3.20)

project(model)

option(TRACE_ON "Debug Trace On" ON)
if (TRACE_ON)
    add_definitions( -DTRACE_ON )
endif()

# source
file(GLOB_RECURSE DIR_SRCS "src/*.cc" "src/*.c")
#file(GLOB_RECURSE DIR_SRCS "src/main.cc")

# head file
include_directories(./src)
include_directories(./thirdParty)
include_directories(./thirdParty/spdlog/include)
include_directories(./thirdParty/fmt/include)
include_directories(./thirdParty/Yaml-cpp/include)

# thirdparty link lib
#include_directories(../thirdparty/yaml-cpp/include)
#link_directories(../thirdparty/yaml-cpp/lib)

# enable c++11
set(CMAKE_CXX_STANDARD 11)

# add_subdirectory()
add_executable(model ${DIR_SRCS})

# link lib
