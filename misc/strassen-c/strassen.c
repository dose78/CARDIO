#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <mkl.h>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>

// C <- A + B
void matrix_add(int m, int n, double *A, int lda, double *B, int ldb, double *C, int ldc) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            C[i*ldc + j] = A[i*lda + j] + B[i*ldb + j];
        }
    }
}

// B <- A + B
void matrix_add_inplace(int m, int n, double *A, int lda, double *B, int ldb) {
    for (int i = 0; i < n; i++) {
        cblas_daxpy(m, 1, A + i*lda, 1, B + i*ldb, 1);
    }
}

// C <- A - B
void matrix_subtract(int m, int n, double *A, int lda, double *B, int ldb, double *C, int ldc) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            C[i*ldc + j] = A[i*lda + j] - B[i*ldb + j];
        }
    }
}

// B <- A - B
void matrix_subtract_inplace(int m, int n, double *A, int lda, double *B, int ldb) {
    for (int i = 0; i < n; i++) {
        cblas_daxpy(m, -1, A + i*lda, 1, B + i*ldb, 1);
    }
}

// Copy B into A
void matrix_copy(int m, int n, double *A, int lda, double *B, int ldb) {
    for (int i = 0; i < n; i++) {
        memcpy(A + i*lda, B + i*ldb, m * sizeof(double));
    }
}

void inner_multiply(int m, int k, int n, double *A, int lda, double *B, int ldb, double *C, int ldc, char* interleaving, int interleaving_length, int depth) {
    if (depth >= interleaving_length) {
        cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, m, n, k, 1, A, lda, B, ldb, 0, C, ldc);
        return;
    }

    int T_m = m/2;
    int T_n = k/2;
    int S_m = k/2;
    int S_n = n/2;

    double *A11 = A;
    double *A21 = A + m/2;
    double *A12 = A + lda*k/2;
    double *A22 = A + lda*k/2 + m/2;

    double *B11 = B;
    double *B21 = B + k/2;
    double *B12 = B + ldb*n/2;
    double *B22 = B + ldb*n/2 + k/2;

    double *C11 = C;
    double *C21 = C + m/2;
    double *C12 = C + ldc*n/2;
    double *C22 = C + ldc*n/2 + m/2;

    double *T0 = A11;
    double *T1 = A12;
    double *T2 = (double*) malloc(T_m * T_n * sizeof(double));
    double *T3 = (double*) malloc(T_m * T_n * sizeof(double));
    double *T4 = (double*) malloc(T_m * T_n * sizeof(double));
    double *T5 = (double*) malloc(T_m * T_n * sizeof(double));
    double *T6 = A22;

    double *S0 = B11;
    double *S1 = B21;
    double *S2 = (double*) malloc(S_m * S_n * sizeof(double));
    double *S3 = (double*) malloc(S_m * S_n * sizeof(double));
    double *S4 = (double*) malloc(S_m * S_n * sizeof(double));
    double *S5 = B22;
    double *S6 = (double*) malloc(S_m * S_n * sizeof(double));

    double *Q0 = C11;
    double *Q1 = (double*) malloc(T_m * S_n * sizeof(double));
    double *Q2 = C22;
    double *Q3 = C12;
    double *Q4 = C21;
    double *Q5 = (double*) malloc(T_m * S_n * sizeof(double));
    double *Q6 = (double*) malloc(T_m * S_n * sizeof(double));

    matrix_add(T_m, T_n, A21, lda, A22, lda, T2, T_m);
    matrix_subtract(T_m, T_n, T2, T_m, A11, lda, T3, T_m);
    matrix_subtract(T_m, T_n, A11, lda, A21, lda, T4, T_m);
    matrix_subtract(T_m, T_n, A12, lda, T3, T_m, T5, T_m);

    matrix_subtract(S_m, S_n, B12, ldb, B11, ldb, S2, S_m);
    matrix_subtract(S_m, S_n, B22, ldb, S2, S_m, S3, S_m);
    matrix_subtract(S_m, S_n, B22, ldb, B12, ldb, S4, S_m);
    matrix_subtract(S_m, S_n, S3, S_m, B21, ldb, S6, S_m);

    if (interleaving[depth] == 'B') {
        cilk_spawn inner_multiply(T_m, T_n, S_n, T0, lda, S0, ldb, Q0, ldc, interleaving, interleaving_length, depth+1);
        cilk_spawn inner_multiply(T_m, T_n, S_n, T1, lda, S1, ldb, Q1, T_m, interleaving, interleaving_length, depth+1);
        cilk_spawn inner_multiply(T_m, T_n, S_n, T2, T_m, S2, S_m, Q2, ldc, interleaving, interleaving_length, depth+1);
        cilk_spawn inner_multiply(T_m, T_n, S_n, T3, T_m, S3, S_m, Q3, ldc, interleaving, interleaving_length, depth+1);
        cilk_spawn inner_multiply(T_m, T_n, S_n, T4, T_m, S4, S_m, Q4, ldc, interleaving, interleaving_length, depth+1);
        cilk_spawn inner_multiply(T_m, T_n, S_n, T5, T_m, S5, ldb, Q5, T_m, interleaving, interleaving_length, depth+1);
        inner_multiply(T_m, T_n, S_n, T6, lda, S6, S_m, Q6, T_m, interleaving, interleaving_length, depth+1);
        cilk_sync;
    } else {
        inner_multiply(T_m, T_n, S_n, T0, lda, S0, ldb, Q0, ldc, interleaving, interleaving_length, depth+1);
        inner_multiply(T_m, T_n, S_n, T1, lda, S1, ldb, Q1, T_m, interleaving, interleaving_length, depth+1);
        inner_multiply(T_m, T_n, S_n, T2, T_m, S2, S_m, Q2, ldc, interleaving, interleaving_length, depth+1);
        inner_multiply(T_m, T_n, S_n, T3, T_m, S3, S_m, Q3, ldc, interleaving, interleaving_length, depth+1);
        inner_multiply(T_m, T_n, S_n, T4, T_m, S4, S_m, Q4, ldc, interleaving, interleaving_length, depth+1);
        inner_multiply(T_m, T_n, S_n, T5, T_m, S5, ldb, Q5, T_m, interleaving, interleaving_length, depth+1);
        inner_multiply(T_m, T_n, S_n, T6, lda, S6, S_m, Q6, T_m, interleaving, interleaving_length, depth+1);
    }

    matrix_add_inplace(m/2, n/2, C11, ldc, C12, ldc);
    matrix_add_inplace(m/2, n/2, C12, ldc, C21, ldc);
    matrix_add_inplace(m/2, n/2, C22, ldc, C12, ldc);
    matrix_add_inplace(m/2, n/2, C21, ldc, C22, ldc);
    matrix_subtract_inplace(m/2, n/2, Q6, m/2, C21, ldc);
    matrix_add_inplace(m/2, n/2, Q5, m/2, C12, ldc);
    matrix_add_inplace(m/2, n/2, Q1, m/2, C11, ldc);

    free(T2);
    free(T3);
    free(T4);
    free(T5);
    free(S2);
    free(S3);
    free(S4);
    free(S6);
    free(Q1);
    free(Q5);
    free(Q6);
}

void multiply(int m, int k, int n, double *A, int lda, double *B, int ldb, double *C, int ldc, char* interleaving, int interleaving_length) {
    inner_multiply(m, k, n, A, lda, B, ldb, C, ldc, interleaving, interleaving_length, 0);
}
