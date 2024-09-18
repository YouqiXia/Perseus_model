cmake_minimum_required (VERSION 3.20)

# enable c++17
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# Define the project name and the language used
project(PERSEUS LANGUAGES CXX)

# Default Build Option -> Debug/Release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build (Debug/Release)" FORCE)
endif()

# Unset C/CXX Flags
set(CMAKE_C_FLAGS "")
set(CMAKE_CXX_FLAGS "")

# Set C/CXX Debug Flags
set(CMAKE_C_FLAGS_DEBUG       "-g -O0")
set(CMAKE_CXX_FLAGS_DEBUG     "-g -O0")

# Set C/CXX Release Flags
set(CMAKE_C_FLAGS_RELEASE     "-g -O3")
set(CMAKE_CXX_FLAGS_RELEASE   "-g -O3")

# Add march flags for x86
add_compile_options("-march=native")

# params for perseus model
set(Perseus_BASE ${CMAKE_CURRENT_SOURCE_DIR})
set(Perseus_LINK_DIR "")
set(Perseus_LIBS "")
set(Perseus_INCLUDES "")
set(Perseus_SRCS "")

# params for pybind lib
set(PybindLib_SRCS "")
set(PybindLib_INCLUDES "")
set(PybindLib_LIBS "")

# set up sparta lib
include(cmake/sparta.cmake)

# set up spike lib
include(cmake/spike.cmake)

# set up STF lib
include(cmake/stf.cmake)

# set up DRAMSim3 lib
include(cmake/DRAMSim3.cmake)

# set up mavis lib
include(cmake/mavis.cmake)

# set up pybind lib
include(cmake/pybind.cmake)

# set up only include file
include(cmake/include_only.cmake)

# source
file(GLOB_RECURSE Perseus_SRCS "src/*.cc" "src/*.cpp" "src/*.c")

# head file
include_directories(./src)
include_directories(./config/cpp_file)

# Create a few links like reports and arch directories
file(CREATE_LINK ${PROJECT_SOURCE_DIR}/reports    ${CMAKE_CURRENT_BINARY_DIR}/reports         SYMBOLIC)
file(CREATE_LINK ${PROJECT_SOURCE_DIR}/arches     ${CMAKE_CURRENT_BINARY_DIR}/arches          SYMBOLIC)
file(CREATE_LINK ${PROJECT_SOURCE_DIR}/config/py_file     ${CMAKE_CURRENT_BINARY_DIR}/config          SYMBOLIC)
file(CREATE_LINK ${PROJECT_SOURCE_DIR}/thirdparty/mavis/json ${CMAKE_CURRENT_BINARY_DIR}/mavis_isa_files SYMBOLIC)
file(CREATE_LINK ${PROJECT_SOURCE_DIR}/traces     ${CMAKE_CURRENT_BINARY_DIR}/traces          SYMBOLIC)
file(CREATE_LINK ${PROJECT_SOURCE_DIR}/src/utils/spikeInterface/default.dtb     ${CMAKE_CURRENT_BINARY_DIR}/default.dtb          SYMBOLIC)

# thirdparty link lib

# add_subdirectory()
add_executable(model ${Perseus_SRCS})

# link lib
target_include_directories(model PUBLIC ${Perseus_INCLUDES})
target_link_directories(model PRIVATE ${Perseus_LINK_DIR})
target_link_libraries(model PRIVATE ${Perseus_LIBS})

# pybind11
pybind11_add_module(unitlib ${PybindLib_SRCS} ${Perseus_SRCS})
set_target_properties(unitlib PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(unitlib PUBLIC ${Perseus_INCLUDES} ${PybindLib_INCLUDES})
target_link_directories(unitlib PRIVATE ${Perseus_LINK_DIR})
target_link_libraries(unitlib PRIVATE ${Perseus_LIBS})
install(TARGETS unitlib DESTINATION ${Perseus_BASE}/lib/python)
