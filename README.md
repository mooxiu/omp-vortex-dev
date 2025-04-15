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
  -DLLVM_ENABLE_PROJECTS="clang;lld;openmp" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=$HOME/llvm \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DLIBOMPTARGET_ENABLE_DEBUG=ON \
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

This means it's very likely that `-Xopenmp-target=vortex` does not work at all. We can add `-ccc-print-phases` to check the compilation phases (I have printed that out in `./simple_offloading_omp/compilation_phases.md`), and it turns out that `m` object, `c` object and `libclang_rt.builtins-riscv64.a` which are supposed to be the input of the device compilation phases are in host compilation phases(phase 2, 3 and 4).


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
-L/home/haruka/llvm/bin/../lib/clang-runtimes/riscv64-unknown-elf/lib 
-L/home/haruka/llvm/lib/clang/18/lib/baremetal 
```

#### Directly pass the arguments through CLW
`clang-linker-wrapper` is responsible for doing the link.
In `clang/tools/clang-linker-wrapper/ClangLinkerWrapper.cpp`, the `main` function will reply on the `CmdArgs` from `Expected<StringRef> clang()` function.

If we hard-coding our paths to the `CmdArgs` by doing:
```
CmdArgs.append("-L/home/haruka/llvm/bin/../lib/clang-runtimes/riscv64-unknown-elf/lib");
CmdArgs.append("-L/home/haruka/llvm/lib/clang/18/lib/baremetal");
```

We will find the file can be compiled.

### The calling track
In fact, CLW is been called twice.
This can be observed by printing the Driver's address inside of Compilation (print `this`).

Usually, in one compilation, the driver will only be called once, but in this specific offloading example, it has been called twice.
By reconstructing the calling chain:
- (1) clang Driver -> call CLW to link
- (2) CLW creates a new clang Driver 
- (3) the new Created clang Driver -> call CLW to link (device code)

By observing the log, we can find our arguments in step (1), but in step (3) it disappers.

Locating where (2) and (3) happens:
- (2): this is done by execute the command line:
```cpp
SmallVector<StringRef, 16> CmdArgs{
    *ClangPath,
    "-o",
    *TempFileOrErr,
    Args.MakeArgString("--target=" + Triple.getTriple()),
    Triple.isAMDGPU() ? Args.MakeArgString("-mcpu=" + Arch)
                      : Args.MakeArgString("-march=" + Arch),
    Args.MakeArgString("-" + OptLevel),
    "-Wl,--no-undefined",
};

//....

if (Error Err = executeCommands(*ClangPath, CmdArgs))
  return std::move(Err);
```

- (3): this is done by `LinkerWrapper::ConstructJob`:
```cpp
 const char *Exec =
      Args.MakeArgString(getToolChain().GetProgramPath("clang-linker-wrapper"));
```

By pass back and forth between the driver and CLW, we can finally make it compiled.

## SEGFAULT: 
After running the compiled binary, I find it fail with SIGSEGV.

Running with debug mode:
```sh
LIBOMPTARGET_DEBUG=1 ./{BINARY_FILE}
```

Checking the log:
```log
omptarget --> Init offload library!
OMPT --> Entering connectLibrary (libomp)
OMPT --> OMPT: Trying to load library libomp.so
OMPT --> OMPT: Trying to get address of connection routine ompt_libomp_connect
OMPT --> OMPT: Library connection handle = 0x730ce91c4b40
OMPT --> Exiting connectLibrary (libomp)
omptarget --> Loading RTLs...
omptarget --> Attempting to load library 'libomptarget.rtl.x86_64.so'...
omptarget --> Successfully loaded library 'libomptarget.rtl.x86_64.so'!
OMPT --> OMPT: Entering connectLibrary (libomptarget)
OMPT --> OMPT: Trying to load library libomptarget.so
OMPT --> OMPT: Trying to get address of connection routine ompt_libomptarget_connect
OMPT --> OMPT: Library connection handle = 0x730ce903bc50
OMPT --> Enter ompt_libomptarget_connect
OMPT --> Leave ompt_libomptarget_connect
OMPT --> OMPT: Exiting connectLibrary (libomptarget)
omptarget --> Registered 'libomptarget.rtl.x86_64.so' with 4 plugin visible devices!
omptarget --> Attempting to load library 'libomptarget.rtl.cuda.so'...
omptarget --> Successfully loaded library 'libomptarget.rtl.cuda.so'!
TARGET CUDA RTL --> Unable to load library 'libcuda.so': libcuda.so: cannot open shared object file: No such file or directory!
TARGET CUDA RTL --> Failed to load CUDA shared library
omptarget --> No devices supported in this RTL
omptarget --> Attempting to load library 'libomptarget.rtl.amdgpu.so'...
omptarget --> Successfully loaded library 'libomptarget.rtl.amdgpu.so'!
TARGET AMDGPU RTL --> Unable to load library 'libhsa-runtime64.so': libhsa-runtime64.so: cannot open shared object file: No such file or directory!
TARGET AMDGPU RTL --> Failed to initialize AMDGPU's HSA library
omptarget --> No devices supported in this RTL
omptarget --> Attempting to load library 'libomptarget.rtl.vortex.so'...
omptarget --> Successfully loaded library 'libomptarget.rtl.vortex.so'!
omptarget --> Registered 'libomptarget.rtl.vortex.so' with 1 plugin visible devices!
omptarget --> RTLs loaded!
omptarget --> Image 0x00005c7dc7f5d0e0 is NOT compatible with RTL libomptarget.rtl.x86_64.so!
omptarget --> Image 0x00005c7dc7f5d0e0 is compatible with RTL libomptarget.rtl.vortex.so!
fish: Job 1, 'LIBOMPTARGET_DEBUG=1 ./host-omp' terminated by signal SIGSEGV (Address boundary error)
```

The problem lies in vortex plugin: `openmp/libomptarget/plugins-nextgen/vortex/src/rtl.cpp`,
method `initImpl` of `VortextDeviceTy` overrides the `GeneralDeviceTy`'s init function,
but failed in `int ret = vx_dev_open(&DeviceHandle);`

```log
haruka@haruka-ubuntu:~/Documents/workspace/vortex/build$ ./ci/blackbox.sh --cores=2 --app=omp_hello
CONFIGS=-DNUM_CORES=2
Running: CONFIGS="-DNUM_CORES=2" make -C ./ci/../runtime/simx > /dev/null
Running: make -C ./ci/../tests/openmp/omp_hello run-simx
make: Entering directory '/home/haruka/Documents/workspace/vortex/build/tests/openmp/omp_hello'
LIBOMPTARGET_DEBUG=1 \
LD_LIBRARY_PATH=/home/haruka/llvm/lib:/home/haruka/Documents/workspace/vortex/build/runtime:/home/haruka/tools/llvm-vortex/lib:/snap/alacritty/149/usr/lib/x86_64-linu
x-gnu/dri  \
VORTEX_DRIVER=simx ./omp_hello
omptarget --> Init offload library!
OMPT --> Entering connectLibrary (libomp)
OMPT --> OMPT: Trying to load library libomp.so
OMPT --> OMPT: Trying to get address of connection routine ompt_libomp_connect
OMPT --> OMPT: Library connection handle = 0x792aaaed0400
OMPT --> Exiting connectLibrary (libomp)
omptarget --> Loading RTLs...
omptarget --> Attempting to load library 'libomptarget.rtl.x86_64.so'...
omptarget --> Successfully loaded library 'libomptarget.rtl.x86_64.so'!
OMPT --> OMPT: Entering connectLibrary (libomptarget)
OMPT --> OMPT: Trying to load library libomptarget.so
OMPT --> OMPT: Trying to get address of connection routine ompt_libomptarget_connect
OMPT --> OMPT: Library connection handle = 0x792aaad1e610
OMPT --> Enter ompt_libomptarget_connect
OMPT --> Leave ompt_libomptarget_connect
OMPT --> OMPT: Exiting connectLibrary (libomptarget)
omptarget --> Registered 'libomptarget.rtl.x86_64.so' with 4 plugin visible devices!
omptarget --> Attempting to load library 'libomptarget.rtl.cuda.so'...
omptarget --> Successfully loaded library 'libomptarget.rtl.cuda.so'!
TARGET CUDA RTL --> Unable to load library 'libcuda.so': libcuda.so: cannot open shared object file: No such file or directory!
TARGET CUDA RTL --> Failed to load CUDA shared library
omptarget --> No devices supported in this RTL
omptarget --> Attempting to load library 'libomptarget.rtl.amdgpu.so'...
omptarget --> Successfully loaded library 'libomptarget.rtl.amdgpu.so'!
TARGET AMDGPU RTL --> Unable to load library 'libhsa-runtime64.so': libhsa-runtime64.so: cannot open shared object file: No such file or directory!
TARGET AMDGPU RTL --> Failed to initialize AMDGPU's HSA library
omptarget --> No devices supported in this RTL
omptarget --> Attempting to load library 'libomptarget.rtl.vortex.so'...
omptarget --> Successfully loaded library 'libomptarget.rtl.vortex.so'!
omptarget --> Registered 'libomptarget.rtl.vortex.so' with 1 plugin visible devices!
omptarget --> RTLs loaded!
omptarget --> Image 0x0000641ebe0290e0 is NOT compatible with RTL libomptarget.rtl.x86_64.so!
omptarget --> Image 0x0000641ebe0290e0 is compatible with RTL libomptarget.rtl.vortex.so!
TARGET VORTEX RTL --> Implementing vx_dev_open with dlsym(vx_dev_open) -> 0x792aab295ac0
TARGET VORTEX RTL --> Implementing vx_dev_close with dlsym(vx_dev_close) -> 0x792aab2959c0
TARGET VORTEX RTL --> Implementing vx_dev_caps with dlsym(vx_dev_caps) -> 0x792aab295a00
TARGET VORTEX RTL --> Implementing vx_mem_alloc with dlsym(vx_mem_alloc) -> 0x792aab295a10
TARGET VORTEX RTL --> Implementing vx_mem_reserve with dlsym(vx_mem_reserve) -> 0x792aab295a20
TARGET VORTEX RTL --> Implementing vx_mem_free with dlsym(vx_mem_free) -> 0x792aab295a30
TARGET VORTEX RTL --> Implementing vx_mem_access with dlsym(vx_mem_access) -> 0x792aab295a40
TARGET VORTEX RTL --> Implementing vx_mem_address with dlsym(vx_mem_address) -> 0x792aab295a50
TARGET VORTEX RTL --> Implementing vx_mem_info with dlsym(vx_mem_info) -> 0x792aab295a60
TARGET VORTEX RTL --> Implementing vx_copy_to_dev with dlsym(vx_copy_to_dev) -> 0x792aab295a70
TARGET VORTEX RTL --> Implementing vx_copy_from_dev with dlsym(vx_copy_from_dev) -> 0x792aab295a80
TARGET VORTEX RTL --> Implementing vx_start with dlsym(vx_start) -> 0x792aab2961a0
TARGET VORTEX RTL --> Implementing vx_ready_wait with dlsym(vx_ready_wait) -> 0x792aab295a90
TARGET VORTEX RTL --> Implementing vx_dcr_read with dlsym(vx_dcr_read) -> 0x792aab295aa0
TARGET VORTEX RTL --> Implementing vx_dcr_write with dlsym(vx_dcr_write) -> 0x792aab295ab0
TARGET VORTEX RTL --> Implementing vx_mpm_query with dlsym(vx_mpm_query) -> 0x792aab296220
TARGET VORTEX RTL --> Implementing vx_upload_kernel_bytes with dlsym(vx_upload_kernel_bytes) -> 0x792aab296420
TARGET VORTEX RTL --> Implementing vx_upload_kernel_file with dlsym(vx_upload_kernel_file) -> 0x792aab299a80
TARGET VORTEX RTL --> Implementing vx_check_occupancy with dlsym(vx_check_occupancy) -> 0x792aab299470
TARGET VORTEX RTL --> Implementing vx_dump_perf with dlsym(vx_dump_perf) -> 0x792aab296690
omptarget --> Plugin adaptor 0x0000641ef9cd8250 has index 0, exposes 1 out of 1 devices!
omptarget --> Registering image 0x0000641ebe0290e0 with RTL libomptarget.rtl.vortex.so!
omptarget --> Done registering entries!
omptarget --> Entering target region for device -1 with entry point 0x0000641ebe02901b
omptarget --> Default TARGET OFFLOAD policy is now mandatory (devices were found)
omptarget --> Use default device id 0
omptarget --> Call to omp_get_num_devices returning 1
omptarget --> Call to omp_get_num_devices returning 1
omptarget --> Call to omp_get_initial_device returning 1
PluginInterface --> Load data from image 0x0000641ebe0290e0
PluginInterface --> Missing symbol __omp_rtl_device_environment, continue execution anyway.
PluginInterface --> Failure to allocate device memory for global memory pool: Failed to allocate from memory manager
PluginInterface --> Skip the memory pool as there is no tracker symbol in the image.Segmentation fault (core dumped)
make: *** [../common.mk:42: run-simx] Error 139
make: Leaving directory '/home/haruka/Documents/workspace/vortex/build/tests/openmp/omp_hello'
```