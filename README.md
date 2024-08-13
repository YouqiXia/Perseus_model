Perseus_model
============================
Risc-V model practice with function & timing split

The basic model based on the Sparta framework has now been established. The functional component of the model is derived from [Spike](https://github.com/riscv-software-src/riscv-isa-sim), while the timing aspect is implemented using [Sparta](https://github.com/sparcians/map/tree/master/sparta).


Build Steps
---------------

We assume that the [Sparta](https://github.com/sparcians/map) environment variable is set to install path. And install all tools in thirdparty.

    $ mkdir build
    $ cd build
    $ cmake ..
    $ make

Compiling and Running an elf or trace file
-------------------------------------------

Running a trace file: 

    $ ./model --run --arch simple --workload ./traces/dhry_riscv.zstf

Running an elf file: 

    $ ./model --run --arch simple --elf ./traces/elf_test/benchmarks/dhrystone.riscv

For more infomation, to see:

    $ ./model --help