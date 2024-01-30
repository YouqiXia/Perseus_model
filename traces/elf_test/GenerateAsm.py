import os
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("--file", type=str)

arg = parser.parse_args()
file = arg.file

asm_file = open(file, "w")

header = [
    "#include \"riscv_test.h\"\n",
    # "#include \"test_macros_vector.h\"\n",
    "\n", "RVTEST_RV64U\n", "RVTEST_CODE_BEGIN\n", "\n"
]

body = [
    "li x30, 64\n"
    "la x29, tdat\n"
    # "csrwi minstret, 0\n"
    # "csrwi mcycle, 0\n"
]

loopnop = [
    "add x0, x0, x0\n",
    "add x0, x0, x0\n",
    "add x0, x0, x0\n"
]

storeloop = [
    "sw x0, 0(x29)\n",
    "sw x0, 0(x29)\n",
    "sw x0, 0(x29)\n"
]

loadloop = [
    "lw x1, 0(x29)\n",
    "lw x1, 0(x29)\n",
    "lw x1, 0(x29)\n"
]

tail = [
    "RVTEST_PASS\n", "\n", ".data\n", "RVTEST_DATA_BEGIN\n", '\n'
]

data = [
    "tdat:\n",
    "tdat1: .word 0x00ff00ff\n",
    "tdat2: .word 0x00ff00ff\n",
    "tdat3: .word 0x00ff00ff\n",
    "tdat4: .word 0x00ff00ff\n",
    "tdat5: .word 0x00ff00ff\n",
    "tdat6: .word 0x00ff00ff\n", "\n"
]

end = ["RVTEST_DATA_END\n"]

asm_file.writelines(header)
asm_file.writelines(body)

for i in range(100000):
    asm_file.writelines(loadloop)

asm_file.writelines(tail)
asm_file.writelines(data)
asm_file.writelines(end)

asm_file.close()