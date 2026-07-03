#include <stdio.h>
#include <stdlib.h>
#include <time.h>


void multiply_matrices_tiled(int N, int tile_size) {
    double *A = (double *)malloc(N * N * sizeof(double));
    double *B = (double *)malloc(N * N * sizeof(double));
    double *C = (double *)malloc(N * N * sizeof(double));

    if (A == NULL || B == NULL || C == NULL) {
        printf("Error: Memory allocation failed!\n");
        return;
    }

    // Initialize with dummy data
    for (int i = 0; i < N * N; i++) {
        A[i] = 1.0;
        B[i] = 2.0;
        C[i] = 0.0;
    }
    
    clock_t start_time = clock();

    for (int ii = 0; ii < N; ii += tile_size) {
        for (int jj = 0; jj < N; jj += tile_size) {
            for (int kk = 0; kk < N; kk += tile_size) {
                
                // Inner loops: Do the normal multiplication, but ONLY up to the edge of the current tile
                for (int i = ii; i < ii + tile_size && i < N; i++) {
                    for (int j = jj; j < jj + tile_size && j < N; j++) {
                        for (int k = kk; k < kk + tile_size && k < N; k++) {
                            C[i * N + j] += A[i * N + k] * B[k * N + j];
                        }
                    }
                }
                
            }
        }
    }

    clock_t end_time = clock();
    double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    printf("%f\n", time_spent);

    free(A);
    free(B);
    free(C);
}

int main(int argc, char *argv[]) {
    // Now we require 3 arguments: ./mat <size> <tile_size>
    if (argc != 3) {
        printf("Usage: %s <matrix_size> <tile_size>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    int tile_size = atoi(argv[2]);

    multiply_matrices_tiled(N, tile_size);

    return 0;
}