# OpenMP Offloading to Vortex GPGPU Dev

## Prepare
- Linux developing environment, or more specifically, Ubuntu or CentOS, as they are the 2 distros which vortex explicitly supports.
- Clone [mooxiu/llvm](https://github.com/mooxiu/llvm/tree/vortex_2.x) and [vortexgpgpu/vortex](https://github.com/vortexgpgpu/vortex).

## SetUp 

### Install the customized LLVM
My command to install LLVM in `$HOME/llvm`:

```sh
cd /your/path/to/llvm
mkdir build && cd build

cmake -G "Ninja" \
  -DLLVM_ENABLE_PROJECTS="clang;openmp;lld" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=$HOME/llvm \
  ../llvm

ninja

ninja install
```

### Install Vortex
Follow the instruction of https://github.com/vortexgpgpu/vortex.

Install Vortex kernel:
```sh
../configure --xlen=64 --tooldir=$HOME/tools --prefix=$HOME/vortex
make -s
make install
```

Vortex kernel and runtime libraries: `$HOME/vortex/`


## Reproduce Linking Issue

Under `simple_offloading_omp` directory, run `make omp_test` can reproduce the same linking error:

```sh
ld.lld: error: unable to find library -lc
ld.lld: error: unable to find library -lm
ld.lld: error: unable to find library -lclang_rt.builtins-riscv64
clang: error: ld.lld command failed with exit code 1 (use -v to see invocation)
${LLVM_INSTALL_PATH}/bin/clang-linker-wrapper: error: 'clang' failed
clang++: error: linker command failed with exit code 1 (use -v to see invocation)
```

### Analysis

The issue happens when `ld.lld` is trying to linking. 

I was thinking this is the last part when host code is linking with device code, but after understanding the offloading process, I realize this error is happened in **device linking**.


After **device linking**, the device object is compiled into a device image, and wrapped with offloading entry... 
only after all of these we enter the last stage of **host linking** the device image and the fat object are linked to make the final binary.

This can be proved by checking the log (running `make omp_test` under `./simple_offloading`):
```sh
"/home/haruka/llvm/bin/clang" -o /tmp/omp_test.riscv64.rv64imafd-02f46b.img --target=riscv64-unknown-elf -march=rv64imafd -O2 -Wl,--no-undefined /tmp/omp_test-051b55-riscv64-unknown-elf-rv64imafd-d4cfd5.o -v
clang version 18.1.7 (git@github.com:mooxiu/llvm.git 3fcb21a600ba8b0b603c93b90db978640c052323)
Target: riscv64-unknown-unknown-elf
Thread model: posix
InstalledDir: /home/haruka/llvm/bin
 "/home/haruka/llvm/bin/ld.lld" --no-undefined /tmp/omp_test-051b55-riscv64-unknown-elf-rv64imafd-d4cfd5.o -Bstatic -L/home/haruka/llvm/bin/../lib/clang-runtimes/riscv64-unknown-elf/lib -L/home/haruka/llvm/bin/../lib/clang-runtimes/riscv64-unknown-elf/lib -L/home/haruka/llvm/lib/clang/18/lib/baremetal -lc -lm -lclang_rt.builtins-riscv64 -X -o /tmp/omp_test.riscv64.rv64imafd-02f46b.img
ld.lld: error: unable to find library -lc
ld.lld: error: unable to find library -lm
ld.lld: error: unable to find library -lclang_rt.builtins-riscv64
clang: error: ld.lld command failed with exit code 1 (use -v to see invocation)
/home/haruka/llvm/bin/clang-linker-wrapper: error: 'clang' failed
clang++: error: linker command failed with exit code 1 (use -v to see invocation)
```
([Complete log](./simple_offloading_omp/docs/omp_test.log))

#### Why device linking fail?

If we search `tools` in the [above log](./simple_offloading_omp/docs/omp_test.log), surprisingly, it is not been mentioned anywhere!

Why this is important? Because library `-lc`(`libc.a`), `-lm`(`libm.a`), `-lclang_rt.builtins-riscv64`(`libclang_rt.builtins-riscv64.a`) should be able to be found in `/home/tools`(where we installed the toolchain), but `ld.lld` was not trying to find them in the places where we assigned to.

To further prove this, I made a simple `hello.cpp` and try to compile it and assigning `-fopenmp -fopenmp-targets=vortex \`, it shows the same result:
```sh
 "/home/haruka/llvm/bin/clang" -o /tmp/hello.riscv64.native-75091e.img --target=riscv64-unknown-elf -march=native -O2 -Wl,--no-undefined /tmp/hello-3f2e24-riscv64-unknown-elf--7dac4d.o -v
clang version 18.1.7 (git@github.com:mooxiu/llvm.git 3fcb21a600ba8b0b603c93b90db978640c052323)
Target: riscv64-unknown-unknown-elf
Thread model: posix
InstalledDir: /home/haruka/llvm/bin
 "/home/haruka/llvm/bin/ld.lld" --no-undefined /tmp/hello-3f2e24-riscv64-unknown-elf--7dac4d.o -Bstatic -L/home/haruka/llvm/bin/../lib/clang-runtimes/riscv64-unknown-elf/lib -L/home/haruka/llvm/bin/../lib/clang-runtimes/riscv64-unknown-elf/lib -L/home/haruka/llvm/lib/clang/18/lib/baremetal -lc -lm -lclang_rt.builtins-riscv64 -X -o /tmp/hello.riscv64.native-75091e.img
ld.lld: error: unable to find library -lc
ld.lld: error: unable to find library -lm
ld.lld: error: unable to find library -lclang_rt.builtins-riscv64
clang: error: ld.lld command failed with exit code 1 (use -v to see invocation)
/home/haruka/llvm/bin/clang-linker-wrapper: error: 'clang' failed
clang++: error: linker command failed with exit code 1 (use -v to see invocation)
```
([Complete_log](./simple_offloading_omp/docs/hello.log))

This means it's very likely that `-Xopenmp-target=vortex` does not work at all. 

But why this does not work and how can we assign to the  `ld.lld` the correct place to find the library?

