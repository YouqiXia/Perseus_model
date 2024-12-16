XLEN ?= 64

RISCV_PREFIX ?= riscv$(XLEN)-unknown-elf-
RISCV_GCC ?= $(RISCV_PREFIX)gcc
RISCV_GCC_OPTS ?= -static -mcmodel=medany -fvisibility=hidden -nostdlib -nostartfiles
RISCV_OBJDUMP ?= $(RISCV_PREFIX)objdump --disassemble-all --disassemble-zeroes --section=.text --section=.text.startup --section=.text.init --section=.data
RISCV_SIM ?= spike

SRC_DIR := ./test
ENV_DIR ?= ./riscv-test-env

ASM_FILES := $(wildcard $(SRC_DIR)/*.S)
P_TESTS := $(patsubst $(SRC_DIR)/%.S, $(SRC_DIR)/%-p.elf, $(ASM_FILES))
V_TESTS := $(patsubst $(SRC DIR)/%.S, $(SRC DIR)/%-v.elf,$(ASM_FILES))
DUMP_FILES := $(patsubst $(SRC_DIR)/%.S, $(SRC_DIR)/%-p.dump, $(ASM_FILES))

default: all

%-p.elf: %.S
	$(RISCV_GCC) -march=rv64gv -mabi=lp64 $(RISCV_GCC_OPTS) -I$(ENV_DIR)/p -T$(ENV_DIR)/p/link.ld $< -o $@

%-v.elf: %.S
	$(RISCV_GCC) -march=rv64gv -mabi=lp64 $(RISCV_GCC_OPTS) -DENTROPY=0x$(shell echo \$$@ | md5sum | cut -c 1-7) -std=gnu99 -O2 -I$(ENV_DIR)/v -T$(ENV_DIR)/p/link.ld $(ENV_DIR)/v/entry.S $(ENV_DIR)/v/*.c $< -o $@

%.dump: %.elf
	$(RISCV_OBJDUMP) $< > $@

p_tests: $(P_TESTS)

v_tests: $(V_TESTS)

dump: $(DUMP_FILES)

.PHONY: p_tests v_tests dump

all: p_tests dump

clean:
	-rm -rf *.elf
	-rm -rf *.dump
