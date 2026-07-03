#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h> 
#include <cblas.h>

void multiply_matrices_tiled(int N, int tile_size, int num_threads) {
    double *A = (double *)malloc(N * N * sizeof(double));
    double *B = (double *)malloc(N * N * sizeof(double));
    double *C = (double *)malloc(N * N * sizeof(double));
    double *C_blas = (double *)malloc(N * N * sizeof(double));

    if (A == NULL || B == NULL || C == NULL || C_blas == NULL) {
        printf("Error: Memory allocation failed!\n");
        return;
    }

    // Initialize matrices
    for (int i = 0; i < N * N; i++) {
        A[i] = 1.0;
        B[i] = 2.0;
        C[i] = 0.0;
        C_blas[i] = 0.0;
    }

    // --- 1. OPENBLAS EXECUTION (BASELINE) ---
    openblas_set_num_threads(num_threads);
    
    double blas_start = omp_get_wtime();
    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, 
                N, N, N, 1.0, A, N, B, N, 0.0, C_blas, N);
    double blas_end = omp_get_wtime();
    double blas_time = blas_end - blas_start;


    // --- 2. CUSTOM TILED EXECUTION (AGENT) ---
    omp_set_num_threads(num_threads);
    
    double omp_start = omp_get_wtime();
    for (int ii = 0; ii < N; ii += tile_size) {
        for (int jj = 0; jj < N; jj += tile_size) {
            for (int kk = 0; kk < N; kk += tile_size) {
                
                for (int i = ii; i < ii + tile_size && i < N; i++) {
                    for (int j = jj; j < jj + tile_size && j < N; j++) {
                        double sum = 0.0; 
                        for (int k = kk; k < kk + tile_size && k < N; k++) {
                            sum += A[i * N + k] * B[k * N + j];
                        }
                        C[i * N + j] += sum;
                    }
                }
            }
        }
    }
    double omp_end = omp_get_wtime();
    double custom_time = omp_end - omp_start;

    // --- CORRECTNESS CHECKSUM ---
    double expected_value = 2.0 * N;
    double actual_value = C[(N * N) - 1]; 
    int is_correct = (actual_value == expected_value) ? 1 : 0;

    // Output format: <custom_time>,<blas_time>,<is_correct>
    printf("%f,%f,%d\n", custom_time, blas_time, is_correct);

    free(A);
    free(B);
    free(C);
    free(C_blas);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <matrix_size> <tile_size> <num_threads>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    int tile_size = atoi(argv[2]);
    int num_threads = atoi(argv[3]);

    multiply_matrices_tiled(N, tile_size, num_threads);
    return 0;
}