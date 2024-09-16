# PYBIND11 as an External Project
# ===========================
# Set up python
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

find_package(pybind11 REQUIRED)

file(GLOB_RECURSE PybindLib_SRCS "config/cpp_file/*.cc" "config/cpp_file/*.cpp" "config/cpp_file/*.c")

list(APPEND PybindLib_LIBS pybind11::headers Python3::Python)

list(APPEND PybindLib_INCLUDES ${Python3_INCLUDE_DIRS})