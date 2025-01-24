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

In `simple_offloading_omp`, run `make omp_test` can reproduce the same linking error:

```sh
ld.lld: error: unable to find library -lc
ld.lld: error: unable to find library -lm
ld.lld: error: unable to find library -lclang_rt.builtins-riscv64
clang: error: ld.lld command failed with exit code 1 (use -v to see invocation)
${LLVM_INSTALL_PATH}/bin/clang-linker-wrapper: error: 'clang' failed
clang++: error: linker command failed with exit code 1 (use -v to see invocation)
```
