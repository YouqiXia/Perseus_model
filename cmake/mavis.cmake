# mavis as an External Project
# ===========================
set (MAVIS_BASE ${Perseus_BASE}/thirdparty/mavis)

add_subdirectory(thirdparty/mavis)
target_compile_options(mavis PRIVATE -fPIC)

list(APPEND Perseus_INCLUDES ${MAVIS_BASE})
list(APPEND Perseus_LIBS mavis)

