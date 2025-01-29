LLVM_PATH=${HOME}/llvm
VORTEX_KERNEL=${HOME}/vortex
VORTEX_HOME=${HOME}/Documents/vortex
TOOLS_PATH=${HOME}/tools


.PHONY: test
test: 	
	${LLVM_PATH}/bin/clang++ \
		-fopenmp \
		-fopenmp-targets=vortex \
		-L/usr/lib/gcc/x86_64-linux-gnu/11 \
		-Xopenmp-target=vortex -invalid-option \
		-v test.c -o test

.PHONY: ttt 
ttt: clean
	${LLVM_PATH}/bin/clang++ \
		-fopenmp \
		-fopenmp-targets=vortex \
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
		-Xopenmp-target=vortex -Wl,-Bstatic,--gc-sections,-T${VORTEX_HOME}/kernel/scripts/link64.ld,--defsym=STARTUP_ADDR=0x80000000 ${VORTEX_HOME}/build/kernel/libvortex.a ${TOOLS_PATH}//riscv64-gnu-toolchain/riscv64-unknown-elf/lib -lm -lc ${TOOLS_PATH}/libcrt64/lib/baremetal/libclang_rt.builtins-riscv64.a \
		simple_offloading.cc \
		-o simple_offloading -v

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
		-Xopenmp-target=vortex -L${TOOLS_PATH}/riscv64-gnu-toolchain/riscv64-unknown-elf/lib -lc -lm \
		-Xopenmp-target=vortex -Wl,-Bstatic,--gc-sections,-T${VORTEX_HOME}/kernel/scripts/link64.ld,--defsym=STARTUP_ADDR=0x80000000 ${VORTEX_HOME}/build/kernel/libvortex.a ${TOOLS_PATH}/libc64/lib/libm.a ${TOOLS_PATH}/libc64/lib/libc.a ${TOOLS_PATH}/libcrt64/lib/baremetal/libclang_rt.builtins-riscv64.a \
		simple_offloading.cc \
		-o simple_offloading -v


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
