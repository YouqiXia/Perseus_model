# DRAMSim3 as an External Project
# ===========================
set(DRAMSim3_DIR "${Perseus_BASE}/thirdparty/DRAMsim3")

add_subdirectory(${DRAMSim3_DIR} DRAMSim3)

list(APPEND Perseus_INCLUDES ${DRAMSim3_DIR}/src)
list(APPEND Perseus_INCLUDES ${DRAMSim3_DIR}/ext/headers)
list(APPEND Perseus_LIBS dramsim3)