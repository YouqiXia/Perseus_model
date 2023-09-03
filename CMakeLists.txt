cmake_minimum_required (VERSION 3.20)

project(model)

# source 
aux_source_directory(. DIR_SRCS)
aux_source_directory(./src DIR_SRCS)

# head file
include_directories(./include)

# thirdparty link lib
include_directories(../thirdparty/yaml-cpp/include)
link_directories(../thirdparty/yaml-cpp/lib)

# enable c++11
set(CMAKE_CXX_STANDARD 11)

# add_subdirectory()
add_executable(model ${DIR_SRCS})

# link lib
target_link_libraries(model yaml-cpp)