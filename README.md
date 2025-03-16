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
  -DLLVM_ENABLE_PROJECTS="clang;lld" \
  -DLLVM_ENABLE_RUNTIMES="openmp" \
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

#### Can we manually run the link?
In the above analysis, the linker fails to find some of libraries, but can we manually link it?

This is failed command, notice that it is trying to find libraries which does not exist:
```sh
 "/home/haruka/llvm/bin/ld.lld" \
   --no-undefined /tmp/omp_test-8e6193-riscv64-unknown-elf-rv64imafd-5e830f.o \
   -Bstatic \
   -L/home/haruka/llvm/bin/../lib/clang-runtimes/riscv64-unknown-elf/lib \
   -L/home/haruka/llvm/bin/../lib/clang-runtimes/riscv64-unknown-elf/lib \
   -L/home/haruka/llvm/lib/clang/18/lib/baremetal \
   -lc -lm -lclang_rt.builtins-riscv64 \
   -X -o /tmp/omp_test.riscv64.rv64imafd-0742e1.img
```

This is the changed:
```sh
> "/home/haruka/llvm/bin/ld.lld" --no-undefined /tmp/omp_test-8e6193-riscv64-unknown-elf-rv64imafd-5e830f.o -Bstatic -L/home/haruka/tools/riscv64-gnu-toolchain/riscv64-unknown-elf/lib -L/home/haruka/tools/libcrt64/lib/baremetal -lc -lm -lclang_rt.builtins-riscv64 -X -o /tmp/omp_test.riscv64.rv64imafd-0742e1.img

# Get this output 
> 
ld.lld: error: /home/haruka/tools/libcrt64/lib/baremetal/libclang_rt.builtins-riscv64.a(muldi3.S.o) is incompatible with /tmp/omp_test-8e6193-riscv64-unknown-elf-rv64imafd-5e830f.o
ld.lld: error: /home/haruka/tools/libcrt64/lib/baremetal/libclang_rt.builtins-riscv64.a(save.S.o) is incompatible with /tmp/omp_test-8e6193-riscv64-unknown-elf-rv64imafd-5e830f.o
ld.lld: error: /home/haruka/tools/libcrt64/lib/baremetal/libclang_rt.builtins-riscv64.a(restore.S.o) is incompatible with /tmp/omp_test-8e6193-riscv64-unknown-elf-rv64imafd-5e830f.o

# Check the ELF
readelf -h /home/haruka/tools/libcrt64/lib/baremetal/libclang_rt.builtins-riscv64.a # simple_offloading_omp/docs/builtins-risc64-elf.log
readelf -h /tmp/omp_test-8e6193-riscv64-unknown-elf-rv64imafd-5e830f.o # simple_offloading_omp/docs/object.log
```

ELF info of the object is aligned with our expectation, but the output of `libclang_rt.builtins-riscv64.a` is more complicated, it has some contents has Class ELF32, which is not compatible with our output (`muldi3.S.o`, `save.S.o` and `restore.S.o`)


## Proposed Partial Solution

### Pass library to ld.lld paths
I have tried many methods and still fail to let ld.lld to use the customized libraries.

The failure part is in linking device code from object file to binary. If we can control this step separately, then we can serve our purpose:

- in the openCL case, PoCL orchestrates the steps behind the scene
- in openMP, clang is trying to do everything in one step and does not provide us space to control each step with fine granularity

To fully solve this problem, we probably need to extend openMP.

But as a partial solution, I'm trying to move our libraries into the default paths ld.lld is searching through.

By checking the error message, we can observe that `ld.lld` will look for `-lc`, `-lc` and `-lclang_rt.builtins-riscv64` in following places:
```sh
-L/home/haruka/llvm/bin/../lib/clang-runtimes/riscv64-unkn
own-elf/lib 
-L/home/haruka/llvm/bin/../lib/clang-runtimes/riscv64-unknown-elf/lib 
-L/home/haruka/llvm/lib/clang/18/lib/baremetal 
```

Then I identified the locations of `libc.a`, `libm.a` and `libclang_rt.builtins-riscv64.a`, and copy the corresponding paths under the searching locations.
```sh
cp -r {HOME}/tools/riscv64-gnu-toolchain/riscv64-unknown-elf/ {HOME}/llvm/lib/clang-runtimes/riscv64-unknown-elf/ #this will include libc.a and libm.a
cp -r {HOME}/tools/libcrt64/lib/ /home/haruka/llvm/lib/clang/18/ # this will inlcude libclang_rt.builtins-riscv64.a
```

Execute again, now we get the same error as above as we try to compile the mid output manually:
```sh
ld.lld: error: /home/haruka/llvm/lib/clang/18/lib/baremetal/libclang_rt.builtins-riscv64.a(muldi3.S.o) is incompatible with /tmp/main.cpp-riscv64-unknown-elf-rv64imaf-391d9a.o
ld.lld: error: /home/haruka/llvm/lib/clang/18/lib/baremetal/libclang_rt.builtins-riscv64.a(save.S.o) is incompatible with /tmp/main.cpp-riscv64-unknown-elf-rv64imaf-391d9a.o
ld.lld: error: /home/haruka/llvm/lib/clang/18/lib/baremetal/libclang_rt.builtins-riscv64.a(restore.S.o) is incompatible with /tmp/main.cpp-riscv64-unknown-elf-rv64imaf-391d9a.o
```

### What Now?

There are 3 alternatives I can think about:
1. Thinking about how to compile `muldi3.S.o`, `save.S.o` and `restore.S.o` into elf64
2. Run the whole thing in riscv32
3. Skip linking `libclang_rt.builtins-risc64.a` 