#include "harness.h"
#include "framework.h"
#include "TestProblem.h"

// void clearCache(double *F) {
//     double sum = 0;
//     for (int i = 0; i < 12500000; i++) {
//         sum += F[i];
//     }
//     if (sum == 0.1) { // Prevent the compiler from optimizing this away
//         printf("sum = %f\n", sum);
//     }
// }

int main(int argc, char **argv) {
    int n = atoi(argv[1]);
    std::string interleaving;
    if (argc > 2) {
        interleaving = argv[2];
    } else {
        interleaving = "";
    }

    double *A = (double*) malloc(n * sizeof(double));
    for(int i = 0; i < n; i++) A[i] = 2 * drand48() - 1;
    TestProblem* problem = new TestProblem(A, n);

    Framework::solve(problem, interleaving);

    // Housekeeping
    free(A);
    delete problem;
    return 0;
}
