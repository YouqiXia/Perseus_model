Perseus_model
============================
Risc-V model practice with function & timing split

The basic model based on the Sparta framework has now been established. The functional component of the model is derived from [Spike](https://github.com/riscv-software-src/riscv-isa-sim), while the timing aspect is implemented using [Sparta](https://github.com/sparcians/map/tree/master/sparta).


Build Steps
---------------

First download and build the [Spike](https://github.com/riscv-software-src/riscv-isa-sim) manually

    git submodule update --init --recursive
    cd thirdparty/riscv-isa-sim/
    ./build.sh

Then Download and install submodules. The first is pybind11, and tag global to install head file of pybind11 in your .local/ 
and make sure .local/ is in your PATH
    
    pip install pybind11[global]

We assume that the [Sparta](https://github.com/sparcians/map) environment variable is set to install path.

    cmake -Bbuild
    cd build
    make

Compiling and Running an elf or trace file
-------------------------------------------

Running a trace file:

    ./model --json arches/config/simple_arch.json --workload ./traces/dhry_riscv.zstf

Running an elf file:

    ./model --json arches/config/simple_arch.json --elf ./traces/elf_test/benchmarks/dhrystone.riscv

For more infomation, to see:

    ./model --help