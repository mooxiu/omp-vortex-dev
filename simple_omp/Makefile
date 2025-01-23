LLVM_PATH=${HOME}/llvm

.PHONY: simple_omp
simple_omp: clean
	${LLVM_PATH}/bin/clang++ \
		-L${LLVM_PATH}/lib \
		-Wl,-rpath,${LLVM_PATH}/lib \
        -I${LLVM_PATH}/include \
		-L/usr/lib/gcc/x86_64-linux-gnu/11 \
		--stdlib=libstdc++ \
		-fopenmp \
		simple_omp.cc \
		-o simple_omp

.PHONY: clean
clean: 
	rm -rf ./simple_omp
