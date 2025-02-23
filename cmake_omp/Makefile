.PHONY: gen-bc
gen-bc:
	/home/haruka/llvm/bin/clang++ -fopenmp -fopenmp-targets=riscv64-unknown-elf \
    -Xopenmp-target=vortex -march=rv64imafd \
	-Xopenmp-target=vortex -mabi=lp64d \
    -emit-llvm -S main.cpp -o main.target.bc

.PHONY: gen32
gen32:
	/home/haruka/llvm/bin/clang++ -fopenmp -fopenmp-targets=riscv32 \
	-Xopenmp-target=riscv32-unknown-elf --sysroot=/home/haruka/tools/riscv32-gnu-toolchain/riscv32-unknown-elf \
	main.cpp -o main -v
