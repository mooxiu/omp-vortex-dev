clang version 18.1.7 (git@github.com:mooxiu/llvm.git 3fcb21a600ba8b0b603c93b90db978640c052323)
Target: x86_64-unknown-linux-gnu
Thread model: posix
InstalledDir: /home/haruka/llvm/bin
Found candidate GCC installation: /usr/lib/gcc/x86_64-linux-gnu/13
Selected GCC installation: /usr/lib/gcc/x86_64-linux-gnu/13
Candidate multilib: .;@m64
Selected multilib: .;@m64
+- 0: input, "/home/haruka/Documents/workspace/vortex/build/kernel/libvortex.a", object, (host-openmp)
|- 1: input, "/home/haruka/tools/riscv64-gnu-toolchain/riscv64-unknown-elf/lib", object, (host-openmp)
|- 2: input, "m", object, (host-openmp)
|- 3: input, "c", object, (host-openmp)
|- 4: input, "/home/haruka/tools/libcrt64/lib/baremetal/libclang_rt.builtins-riscv64.a", object, (host-openmp)
|              +- 5: input, "omp_test.cpp", c++, (host-openmp)
|           +- 6: preprocessor, {5}, c++-cpp-output, (host-openmp)
|        +- 7: compiler, {6}, ir, (host-openmp)
|        |              |     +- 8: input, "omp_test.cpp", c++, (device-openmp)
|        |              |  +- 9: preprocessor, {8}, c++-cpp-output, (device-openmp)
|        |              |- 10: compiler, {9}, ir, (device-openmp)
|        |           +- 11: offload, "host-openmp (x86_64-unknown-linux-gnu)" {7}, "device-openmp (riscv64-unknown-unknown-elf)" {10}, ir
|        |        +- 12: backend, {11}, assembler, (device-openmp)
|        |     +- 13: assembler, {12}, object, (device-openmp)
|        |  +- 14: offload, "device-openmp (riscv64-unknown-unknown-elf)" {13}, object
|        |- 15: clang-offload-packager, {14}, image, (device-openmp)
|     +- 16: offload, "host-openmp (x86_64-unknown-linux-gnu)" {7}, "device-openmp (x86_64-unknown-linux-gnu)" {15}, ir
|  +- 17: backend, {16}, assembler, (host-openmp)
|- 18: assembler, {17}, object, (host-openmp)
19: clang-linker-wrapper, {0, 1, 2, 3, 4, 18}, image, (host-openmp)
