LLVM_PATH=${HOME}/llvm
VORTEX_KERNEL=${HOME}/vortex
VORTEX_HOME=${HOME}/Documents/vortex
TOOLS_PATH=${HOME}/tools

.PHONY: omp_test
omp_test: clean
	${LLVM_PATH}/bin/clang++ \
		-fopenmp \
		-fopenmp-targets=vortex \
		-Wl,-rpath,/usr/lib/gcc/x86_64-linux-gnu/11 \
		-L/usr/lib/gcc/x86_64-linux-gnu/11 \
		-Xopenmp-target=vortex --sysroot=${TOOLS_PATH}/riscv64-gnu-toolchain/riscv64-unknown-elf \
		-Xopenmp-target=vortex --gcc-toolchain=${TOOLS_PATH}/riscv64-gnu-toolchain \
		-Xopenmp-target=vortex -march=rv64imafd \
		-Xopenmp-target=vortex -mabi=lp64d \
		-Xopenmp-target=vortex -mcmodel=medany \
		-Xopenmp-target=vortex -fno-rtti \
		-Xopenmp-target=vortex -fno-exceptions \
		-Xopenmp-target=vortex -nostartfiles \
		-Xopenmp-target=vortex -nostdlib \
		-Xopenmp-target=vortex -fdata-sections \
		-Xopenmp-target=vortex -ffunction-sections \
		-Xopenmp-target=vortex -I${VORTEX_KERNEL}/kernel/include \
		-Xopenmp-target=vortex -L${TOOLS_PATH}/libc64/lib \
		-Xopenmp-target=vortex -L${TOOLS_PATH}/libcrt64/lib/baremetal \
		-Xopenmp-target=vortex -lm \
		-Xopenmp-target=vortex -lc \
		-Xopenmp-target=vortex -lclang_rt.builtins-riscv64.a \
		-Xopenmp-target=vortex -Wl,-Bstatic,--gc-sections,-T${VORTEX_HOME}/kernel/scripts/link64.ld,--defs \
		simple_offloading.cc \
		-o simple_offloading


.PHONY: simple_offloading
simple_offloading: clean
	${LLVM_PATH}/bin/clang++ \
		-L${LLVM_PATH}/lib \
		-Wl,-rpath,${LLVM_PATH}/lib \
        -I${LLVM_PATH}/include \
		-L/usr/lib/gcc/x86_64-linux-gnu/11 \
		--stdlib=libstdc++ \
		-fopenmp \
		simple_offloading.cc \
		-o simple_offloading

.PHONY: clean
clean: 
	rm -rf ./simple_offloading
