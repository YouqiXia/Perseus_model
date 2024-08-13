import os
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("--file", type=str)

arg = parser.parse_args()
file = arg.file

asm_file = open(file, "w")

inner_loop_num = 20

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
    "sw x30, 0(x29)\n",
    "sw x30, 0(x29)\n",
    "sw x30, 0(x29)\n"
]

loadloop = [
    "lw x1, 0(x29)\n",
    "lw x1, 0(x29)\n",
    "lw x1, 0(x29)\n"
]

dependencyloop = [
    "add x1,  x0,  x0 \n",
    "add x2,  x1,  x0 \n",
    "add x3,  x2,  x1 \n",
    "add x4,  x3,  x2 \n",
    "add x5,  x4,  x3 \n",
    "add x6,  x5,  x4 \n",
    "add x7,  x6,  x5 \n",
    "add x8,  x7,  x6 \n",
    "add x9,  x8,  x7 \n",
    "add x10, x9,  x8 \n",
    "add x11, x10, x9 \n",
    "add x12, x11, x10\n",
    "add x13, x12, x11\n",
    "add x14, x13, x12\n",
    "add x15, x14, x13\n",
    "add x16, x15, x14\n",
    "add x17, x16, x15\n",
    "add x18, x17, x16\n"
]

tail = [
    "RVTEST_PASS\n", "\n", ".data\n", "RVTEST_DATA_BEGIN\n", '\n'
]

data = ["tdat:\n",]

def generate_string_array(n):
    string_array = [f'tdat{i}: .word 0x00000000\n' for i in range(n)]
    return data + string_array

def generate_storeloop(n):
    string_array = [f'sw x30, {4*i}(x29)\n' for i in range(n)]
    return string_array


end = ["RVTEST_DATA_END\n"]

asm_file.writelines(header)
asm_file.writelines(body)

for i in range(10000):
    asm_file.writelines(dependencyloop)

asm_file.writelines(tail)
asm_file.writelines(generate_string_array(inner_loop_num))
asm_file.writelines(end)

asm_file.close()
