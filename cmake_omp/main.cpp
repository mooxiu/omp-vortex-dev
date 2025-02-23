// Try not to import anything

// #include <iostream>
#include "omp.h"
#include <stdio.h>


#define SIZE 32

int main() {
    int i;
    int A [SIZE], B [SIZE], C[SIZE];

    // initiate A and B
    for (i = 0; i < SIZE; i++) {
        A[i] = i;
        B[i] = 2 * i;
    }

    // C[i] = A[i] + B[i]
    #pragma omp target teams distribute parallel for map(to: A[0:SIZE], B[0:SIZE]) map(from: C[0:SIZE])
    for (i = 0; i < SIZE; i++) {
        // std::cout << "get " << omp_get_thread_num() << std::endl;
        C[i] = A[i] + B[i];
    }

    // TODO: comment this after testing
    for (i = 0; i < SIZE; i++) {
      printf("%d\n", C[i]);
    }
    
    return 0;
}