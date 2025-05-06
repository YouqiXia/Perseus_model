set(srcs
    UnitBuilder.cpp
)
add_library(unitbuild ${srcs})
set_target_properties(unitbuild PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(unitbuild PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)
target_include_directories(unitbuild PRIVATE ..)
target_link_libraries(unitbuild unit_registered)