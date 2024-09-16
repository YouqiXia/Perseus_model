# ELFIO as an External include only Project
# ===========================
set (ELFIO_BASE ${Perseus_BASE}/thirdparty/ELFIO)

add_subdirectory(thirdparty/ELFIO)

list(APPEND Perseus_INCLUDES ${ELFIO_BASE})

# json as an External include only Project
# ===========================
set (EXT_BASE ${Perseus_BASE}/thirdparty/head_file_only)

list(APPEND Perseus_INCLUDES ${EXT_BASE})
