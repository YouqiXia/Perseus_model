set(src
    UnitRegister.cpp
    DoAllRegister.cpp
    example.cpp
    FE.cpp
    BE.cpp
    Extension.cpp
)

# registered_srcs is a collection of all user source files
set(registered_srcs
    ${registered_libs}
)

# registered_libs is a collection of all user lib
set(registered_libs
    ${registered_libs}
)

# All needed include path
set(registered_include_path
    ${registered_include_path}
)

# All needed link path
set(registered_link_path
    ${registered_link_path}
)

add_library(unit_registered ${src} ${registered_srcs} ${Perseus_SRCS})
set_target_properties(unit_registered PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(unit_registered PUBLIC ${Perseus_LIBS} ${registered_libs})
target_link_directories(unit_registered PUBLIC ${registered_link_path} ${Perseus_LINK_DIR})
target_include_directories(unit_registered PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${registered_include_path}
    ${Perseus_INCLUDES}
)