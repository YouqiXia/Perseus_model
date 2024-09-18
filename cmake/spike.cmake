# spike as an External Project
# ===========================
set (SPIKE_BASE ${Perseus_BASE}/thirdparty/riscv-isa-sim)

list(APPEND Perseus_INCLUDES ${Perseus_BASE}/src/utils/spikeInterface/src)
list(APPEND Perseus_INCLUDES ${SPIKE_BASE})
list(APPEND Perseus_INCLUDES ${SPIKE_BASE}/libs/include)
list(APPEND Perseus_INCLUDES ${SPIKE_BASE}/libs/include/riscv)
list(APPEND Perseus_INCLUDES ${SPIKE_BASE}/libs/include/fesvr)
list(APPEND Perseus_INCLUDES ${SPIKE_BASE}/libs/include/fdt)
list(APPEND Perseus_INCLUDES ${SPIKE_BASE}/libs/include/softfloat)
list(APPEND Perseus_INCLUDES ${SPIKE_BASE}/riscv)

set (SPIKE_INTF_LIBS riscv softfloat fesvr disasm fdt boost_regex)
list(APPEND Perseus_LINK_DIR ${SPIKE_BASE}/libs/lib)
list(APPEND Perseus_LIBS ${SPIKE_INTF_LIBS})
