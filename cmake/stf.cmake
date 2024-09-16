# stf as an External Project
# ===========================
set (STF_LIB_BASE ${Perseus_BASE}/thirdparty/stf_lib)
set (DISABLE_STF_DOXYGEN ON)

if (CMAKE_BUILD_TYPE MATCHES "^[Rr]elease")
    set (FULL_LTO true)
    include(${STF_LIB_BASE}/cmake/stf_linker_setup.cmake)
    setup_stf_linker(false)
endif()

add_subdirectory (${STF_LIB_BASE})

list(APPEND Perseus_INCLUDES ${STF_LIB_BASE})
list(APPEND Perseus_LIBS ${STF_LINK_LIBS})

