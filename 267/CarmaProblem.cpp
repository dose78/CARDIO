#include "CarmaProblem.h"

int CarmaProblem::getDimToSplit() {
    if (n >= k && n >= m) return SPLIT_N;
    if (m >= k && m >= n) return SPLIT_M;
    return SPLIT_K;
}

CarmaProblem::CarmaProblem(int M, int K, int m, int k, int n, int CM, double *A, double *B, double *C) {
    this->M = M; this->K = K; this->CM = CM;
    this->m = m; this->k = k; this->n = n;
    this->A = A; this->B = B; this->C = C;
}

bool CarmaProblem::shouldRunBaseCase(int depth) {
    return (depth >= MAX_DEPTH);
}

void CarmaProblem::runBaseCase() {
    cblas_dgemm(CblasColMajor,CblasNoTrans,CblasNoTrans,m,n,k,1,A,M,B,K,0,C,CM);
}

std::vector<Task*> CarmaProblem::split() {
    Task* task1 = new Task();
    Task* task2 = new Task();
    int dim = getDimToSplit();
    if (dim == SPLIT_N) {
        double *B1 = B;
        double *B2 = B + n/2*K;
        task1->addProblem(new CarmaProblem(M, K, m, k, n/2, CM, A, B1, C));
        task2->addProblem(new CarmaProblem(M, K, m, k, n/2, CM, A, B2, C + n/2*CM));
    } else if (dim == SPLIT_M) {
        double *A1 = A;
        double *A2 = A + m/2;
        task1->addProblem(new CarmaProblem(M, K, m/2, k, n, CM, A1, B, C));
        task2->addProblem(new CarmaProblem(M, K, m/2, k, n, CM, A2, B, C + m/2));
    } else { // SPLIT_K
        double *A1 = A;
        double *A2 = A + k/2*M;
        double *B1 = B;
        double *B2 = B + k/2;
        double *Q1 = (double*) malloc(m * n * sizeof(double));
        task1->addProblem(new CarmaProblem(M, K, m, k/2, n, m, A1, B1, Q1));
        task2->addProblem(new CarmaProblem(M, K, m, k/2, n, CM, A2, B2, C));
    }

    std::vector<Task*> tasks;
    tasks.push_back(task1);
    tasks.push_back(task2);
    return tasks;
}

void CarmaProblem::merge(std::vector<Problem*> subproblems) {
    int dim = getDimToSplit();
    if (dim == SPLIT_K) {
        CarmaProblem* subproblem = static_cast<CarmaProblem*>(subproblems[0]);
        for (int x = 0; x < subproblem->n; x++) {
            cblas_daxpy(subproblem->m, 1, subproblem->C + subproblem->m * x, 1, C + CM * x, 1);
        }
        free(subproblem->C);
    }
}
