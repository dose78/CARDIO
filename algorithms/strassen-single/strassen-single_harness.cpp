#include "harness.h"
#include "StrassenSingleProblem.h"
// #include "StrassenSingleNaiveProblem.h"
#include "framework.h"

void initialize(int m, int k, int n, float* A, float* B, float* C) {
    for(int i = 0; i < m*k; i++) A[i] = 2 * drand48() - 1;
    for(int i = 0; i < k*n; i++) B[i] = 2 * drand48() - 1;
    for(int i = 0; i < m*n; i++) C[i] = 2 * drand48() - 1;
}

int main(int argc, char **argv) {
    srand48(time(NULL));

    int m = atoi(argv[1]);
    int k = atoi(argv[2]);
    int n = atoi(argv[3]);

    FILE *f = fopen("strassen-single.csv","a");

    float *A, *B, *C;
    A = (float*) malloc(m * k * sizeof(float));
    B = (float*) malloc(k * n * sizeof(float));
    C = (float*) malloc(m * n * sizeof(float));
    initialize(m, k, n, A, B, C);
    StrassenSingleProblem* problem = new StrassenSingleProblem(m, k, n, A, m, B, k, C, m);
    // StrassenSingleNaiveProblem* problem = new StrassenSingleNaiveProblem(m, k, n, A, B, C);

    // Time multiplication
    struct timeval start, end;
    gettimeofday(&start, NULL);
    solve(problem);
    gettimeofday(&end, NULL);
    double seconds = (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
    double Gflop_s = 2e-9 * m * k * n / seconds;

    if (seconds < 0.02) {
        printf("WARNING: Matrix size may be too small to produce accurate timing data\n");
    }

    fprintf(f,"STRASSEN-SINGLE: %d,%d,%d,%f\n", m, k, n, Gflop_s);
    printf("STRASSEN-SINGLE: %d,%d,%d,%f\n", m, k, n, Gflop_s);

    // check for correctness
    // cblas_sgemm(CblasColMajor,CblasNoTrans,CblasNoTrans, m,n,k, -1, A,m, B,k, 1, C,m);
    // for(int i = 0; i < m*k; i++) A[i] = fabs( A[i] );
    // for(int i = 0; i < k*n; i++) B[i] = fabs( B[i] );
    // for(int i = 0; i < m*n; i++) C[i] = fabs( C[i] );
    // cblas_sgemm(CblasColMajor,CblasNoTrans,CblasNoTrans, m,n,k, -3.0*FLT_EPSILON*n, A,m, B,k, 1, C,m);
    // for(int i = 0; i < m*n; i++) {
    //     if(C[i] > 0) {
    //         printf("FAILURE: error in matrix multiply exceeds an acceptable margin\n");
    //         return -1;
    //     }
    // }

    // Housekeeping
    free(A);
    free(B);
    free(C);
    delete problem;
    fclose(f);
    #ifdef DEBUG
        printf("memory: %d bytes\n", Memory::current);
        printf("max: %d bytes\n", Memory::max);
    #endif
    return 0;
}
