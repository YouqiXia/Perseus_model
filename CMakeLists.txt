cmake_minimum_required (VERSION 3.20)
# enable c++17
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

project(model)

# set sparta directory
set(SPARTA_SEARCH_DIR /usr/local)
set(SPARTA_INCLUDE_DIRS ${SPARTA_SEARCH_DIR}/include/sparta)
set(CMAKE_MODULE_PATH "${SPARTA_SEARCH_DIR}/lib/cmake/sparta" ${CMAKE_MODULE_PATH})
find_package(Sparta REQUIRED)

# set up STF lib
set (STF_LIB_BASE ${PROJECT_SOURCE_DIR}/stf_lib)
set (DISABLE_STF_DOXYGEN ON)

if (CMAKE_BUILD_TYPE MATCHES "^[Rr]elease")
  set (FULL_LTO true)
  include(${STF_LIB_BASE}/cmake/stf_linker_setup.cmake)
  setup_stf_linker(false)
endif()

# source
file(GLOB_RECURSE DIR_SRCS "src/*.cc" "src/*.cpp" "src/*.c")

# head file
include_directories(./src)
include_directories(./src/basic)
include_directories(./src/uncore)
include_directories(mavis)
include_directories(stf_lib)
include_directories(${SPARTA_INCLUDE_DIRS})

# Profile build flags
set(CMAKE_CXX_FLAGS_PROFILE   "-O3 -pg -g -ftime-trace")
set(CMAKE_CXX_FLAGS_FASTDEBUG "-O3 -g")
set(CMAKE_CXX_FLAGS_DEBUG     "-O0 -g")

# Create a few links like reports and arch directories
file(CREATE_LINK ${PROJECT_SOURCE_DIR}/reports    ${CMAKE_CURRENT_BINARY_DIR}/reports         SYMBOLIC)
file(CREATE_LINK ${PROJECT_SOURCE_DIR}/arches     ${CMAKE_CURRENT_BINARY_DIR}/arches          SYMBOLIC)
file(CREATE_LINK ${PROJECT_SOURCE_DIR}/mavis/json ${CMAKE_CURRENT_BINARY_DIR}/mavis_isa_files SYMBOLIC)
file(CREATE_LINK ${PROJECT_SOURCE_DIR}/traces     ${CMAKE_CURRENT_BINARY_DIR}/traces          SYMBOLIC)

# thirdparty link lib
add_subdirectory(mavis)
add_subdirectory (${STF_LIB_BASE})

# add_subdirectory()
add_executable(model ${DIR_SRCS})

# link lib
target_link_libraries(model SPARTA::sparta mavis ${STF_LINK_LIBS})
