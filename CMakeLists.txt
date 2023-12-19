cmake_minimum_required (VERSION 3.20)
# enable c++17
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

project(model)

# set sparta directory
set(SPARTA_SEARCH_DIR /home/yzhang/.local/)
set(SPARTA_INCLUDE_DIRS /home/yzhang/.local/include/sparta)
set(CMAKE_MODULE_PATH "${SPARTA_SEARCH_DIR}/lib/cmake/sparta" ${CMAKE_MODULE_PATH})
find_package(Sparta REQUIRED)

# source
file(GLOB_RECURSE DIR_SRCS "src/*.cc" "src/*.cpp" "src/*.c")

# head file
include_directories(./src)
include_directories(${SPARTA_INCLUDE_DIRS})

# thirdparty link lib

# add_subdirectory()
add_executable(model ${DIR_SRCS})

# link lib
target_link_libraries(model SPARTA::sparta)
