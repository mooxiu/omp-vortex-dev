cmake_minimum_required(VERSION 3.18)
project(OMP_Vortex LANGUAGES C CXX)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# OpenMP Offloading Settings
find_package(OpenMP REQUIRED)

# Vortex environment variables
set(TOOLDIR $ENV{HOME}/tools)
set(VORTEX_HOME $ENV{HOME}/Documents/workspace/vortex)
set(VORTEX_BUILD $ENV{HOME}/Documents/workspace/vortex/build)
set(STARTUP_ADDR 0x80000000)
set(LLVM_VORTEX $ENV{HOME}/llvm)  # Path to custom LLVM for Vortex
# set(RISCV_SYSROOT ${TOOLDIR}/riscv64-gnu-toolchain/riscv64-unknown-elf)
set(RISCV_SYSROOT ${TOOLDIR}/riscv64-gnu-toolchain/riscv64-unknown-elf)
set(VORTEX_TRIPLE "riscv64-unknown-elf")

# Set the system name to indicate cross-compiling
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR riscv64) # 32?

# Specify the binary utilities
# TODO: probably not use
set(CMAKE_AR "${TOOLDIR}/llvm-vortex/bin/llvm-ar")
#set(CMAKE_ASM_COMPILER "${TOOLDIR}/llvm-vortex/bin/llvm-as")
set(CMAKE_LINKER "${TOOLDIR}/llvm-vortex/bin/llvm-lld")
set(CMAKE_NM "${TOOLDIR}/llvm-vortex/bin/llvm-nm")
set(CMAKE_RANLIB "${TOOLDIR}/llvm-vortex/bin/llvm-ranlib")


# TODO: choose which one?
# Specify the compilers
# set(CMAKE_C_COMPILER "${TOOLDIR}/llvm-vortex/bin/clang")
# set(CMAKE_CXX_COMPILER "${TOOLDIR}/llvm-vortex/bin/clang++")
set(CMAKE_C_COMPILER "${LLVM_VORTEX}/bin/clang")
set(CMAKE_CXX_COMPILER "${LLVM_VORTEX}/bin/clang++")


# Compiler flags for C and C++
set(CMAKE_CXX_FLAGS "-fopenmp -fopenmp-targets=${VORTEX_TRIPLE}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xopenmp-target=${VORTEX_TRIPLE} -nostdlib") 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xopenmp-target=${VORTEX_TRIPLE} -nodefaultlibs") 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xopenmp-target=${VORTEX_TRIPLE} -nostartfiles") 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xopenmp-target=${VORTEX_TRIPLE} --sysroot=${RISCV_SYSROOT}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xopenmp-target=${VORTEX_TRIPLE} --gcc-toolchain=${TOOLDIR}/riscv-gnu-toolchain")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xopenmp-target=${VORTEX_TRIPLE} -march=rv64imaf")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xopenmp-target=${VORTEX_TRIPLE} -mabi=lp64d")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xopenmp-target=${VORTEX_TRIPLE} -L${RISCV_SYSROOT}/lib")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xopenmp-target=${VORTEX_TRIPLE} -lc -lm")


# set(CMAKE_SYSROOT ${RISCV_SYSROOT})

# set(CMAKE_C_FLAGS "-v --gcc-toolchain=${TOOLDIR}/riscv-gnu-toolchain -march=rv64imaf -mabi=ilp64f -Xclang -target-feature -Xclang +vortex -mcmodel=medany -fno-rtti -fno-exceptions -fdata-sections -ffunction-sections")
# set(CMAKE_CXX_FLAGS "-v --gcc-toolchain=${TOOLDIR}/riscv-gnu-toolchain -march=rv64imaf -mabi=ilp64f -Xclang -target-feature -Xclang +vortex -mcmodel=medany -fno-rtti -fno-exceptions -fdata-sections -ffunction-sections")


# set(CMAKE_CXX_FLAGS 
#     "-fopenmp \
#     -fopenmp-targets=vortex \
#     -Xopenmp-target=vortex --sysroot=${RISCV_SYSROOT} \
#     -Xopenmp-target=vortex -march=rv64imafd \
#     -Xopenmp-target=vortex -mabi=lp64d \
#     -Xopenmp-target=vortex -L${RISCV_SYSROOT}/lib \
#     -Xopenmp-target=vortex -lc -lm \
#     -Xopenmp-target=vortex -L${RISCV_TOOLCHAIN_PATH}/libcrt64/lib/baremetal ${RISCV_TOOLCHAIN_PATH}/libcrt64/lib/baremetal/libclang_rt.builtins-riscv64.a \
#     -Xopenmp-target=vortex -L${VORTEX_HOME}/build/runtime \
#     -Xopenmp-target=vortex -lvortex \
#     -Xopenmp-target=vortex -Wl,-Bstatic,--gc-sections,-T${VORTEX_HOME}/kernel/scripts/link64.ld \
#     "
# )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")


# Include paths
include_directories(${VORTEX_HOME}/kernel/include ${VORTEX_HOME}/runtime)

# Linker Flags for Vortex

message(STATUS "RISCV_TOOLCHAIN_PATH is set to: ${RISCV_TOOLCHAIN_PATH}")


# Add the executable
add_executable(omp_vortex main.cpp)

# Link OpenMP and Vortex runtime
target_link_libraries(omp_vortex OpenMP::OpenMP_CXX)

# Linker flags for host linking only
# set(CMAKE_EXE_LINKER_FLAGS 
#     "-Wl,--gc-sections \
#     -Wl,-Bstatic -Xopenmp-target=vortex --defsym=STARTUP_ADDR=0x80000001 \
#     -Wl,-Bdynamic")


set(CMAKE_EXE_LINKER_FLAGS 
    "-Wl,--gc-sections \
    -Xopenmp-target=${VORTEX_TRIPLE} --defsym=STARTUP_ADDR=0x80000001 \
    ")



# set(CMAKE_EXE_LINKER_FLAGS "-Wl,-Bstatic,--gc-sections,-T${VORTEX_HOME}/kernel/scripts/link64.ld \
                            # -Xopenmp-target=vortex --defsym=STARTUP_ADDR=0x80000001 -L${VORTEX_HOME}/runtime -lvortex \
                            # -Xopenmp-target=vortex -L${RISCV_TOOLCHAIN_PATH}/libcrt64/lib/baremetal ${RISCV_TOOLCHAIN_PATH}/libcrt64/lib/baremetal/libclang_rt.builtins-riscv64.a")





# Target properties for OpenMP Offloading
set_target_properties(omp_vortex PROPERTIES LINKER_LANGUAGE CXX)

