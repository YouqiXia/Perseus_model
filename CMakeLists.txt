cmake_minimum_required (VERSION 3.20)

project(model)

# source
file(GLOB_RECURSE DIR_SRCS "src/*.cc" "src/*.c")

# head file
include_directories(./src/include)

# thirdparty link lib
#include_directories(../thirdparty/yaml-cpp/include)
#link_directories(../thirdparty/yaml-cpp/lib)

# enable c++11
set(CMAKE_CXX_STANDARD 11)

# add_subdirectory()
add_executable(model ${DIR_SRCS})

# link lib
