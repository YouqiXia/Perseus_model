set(src
    UnitRegister.cpp
    DoAllRegister.cpp
    example.cpp
    FE.cpp
    BE.cpp
)

add_library(unit_registered ${src} ${Perseus_SRCS})
set_target_properties(unit_registered PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(unit_registered PUBLIC ${Perseus_LIBS})
target_link_directories(unit_registered PUBLIC ${Perseus_LINK_DIR})
target_include_directories(unit_registered PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${Perseus_INCLUDES}
)