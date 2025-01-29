LLVM_PATH=${HOME}/llvm
VORTEX_KERNEL=${HOME}/vortex
VORTEX_HOME=${HOME}/Documents/workspace/vortex
TOOLS_PATH=${HOME}/tools


.PHONY: hello 
hello: 	
	${LLVM_PATH}/bin/clang++ \
		-fopenmp \
		-fopenmp-targets=vortex \
		-Xopenmp-target=vortex -invalid-option \
		-v hello.cpp -o hello 

.PHONY: omp_test 
omp_test: clean
	${LLVM_PATH}/bin/clang++ \
		-fopenmp \
		-fopenmp-targets=vortex \
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
		-Xopenmp-target=vortex -Wl,-Bstatic,--gc-sections,-T${VORTEX_HOME}/kernel/scripts/link64.ld,--defsym=STARTUP_ADDR=0x80000000 ${VORTEX_HOME}/build/kernel/libvortex.a ${TOOLS_PATH}/riscv64-gnu-toolchain/riscv64-unknown-elf/lib -lm -lc ${TOOLS_PATH}/libcrt64/lib/baremetal/libclang_rt.builtins-riscv64.a \
		omp_test.cpp \
		-o omp_test -v

.PHONY: clean
clean: 
	rm -rf ./simple_offloading
