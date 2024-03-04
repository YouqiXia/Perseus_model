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

for i in range(10):
    asm_file.writelines(generate_storeloop(inner_loop_num))

asm_file.writelines(tail)
asm_file.writelines(generate_string_array(inner_loop_num))
asm_file.writelines(end)

asm_file.close()
