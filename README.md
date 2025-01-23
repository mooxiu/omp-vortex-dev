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
  -DLLVM_ENABLE_PROJECTS="clang;openmp" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=$HOME/llvm \
  ../llvm

ninja

ninja install
```

### Install Vortex
Follow the instruction of https://github.com/vortexgpgpu/vortex.


## Reproduce Linking Issue
