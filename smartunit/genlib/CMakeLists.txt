set(srcs
    UnitLib.cpp
)

pybind11_add_module(UnitLib ${srcs})
set_target_properties(UnitLib PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(UnitLib PRIVATE unit_registered SPARTA::sparta)

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/unitlib/lib)
message("==============================")
message("UnitLib output path: ${LIBRARY_OUTPUT_PATH}")
message("==============================")