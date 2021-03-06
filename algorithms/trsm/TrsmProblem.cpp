#include "MultProblem.h"
#include "TrsmProblem.h"
#include "memorytracking.h"

TrsmProblem::TrsmProblem(double *X, double *T, int n, int N) {
    this->X = X;
    this->T = T;
    this->n = n;
    this->N = N;
}

void TrsmProblem::runBaseCase() {
    cblas_dtrsm(CblasColMajor, CblasRight, CblasLower, CblasTrans, CblasNonUnit, n, n, 1.0, T, N, X, N);
}

std::vector<Task*> TrsmProblem::split() {
    double* X11 = X;
    double* X12 = X + N*n/2;
    double* X21 = X + n/2;
    double* X22 = X + N*n/2 + n/2;

    double* T11 = T;
    double* T12 = T + N*n/2;
    double* T21 = T + n/2;
    double* T22 = T + N*n/2 + n/2;

    Task* task1 = new Task(3);
    task1->addProblem(new TrsmProblem(X11, T11, n/2, N));
    task1->addProblem(new MultProblem(X12, X11, T21, n/2, N, N, N));
    task1->addProblem(new TrsmProblem(X12, T22, n/2, N));

    Task* task2 = new Task(3);
    task2->addProblem(new TrsmProblem(X21, T11, n/2, N));
    task2->addProblem(new MultProblem(X22, X21, T21, n/2, N, N, N));
    task2->addProblem(new TrsmProblem(X22, T22, n/2, N));

    std::vector<Task*> tasks (2);
    tasks[0] = task1;
    tasks[1] = task2;
    return tasks;
}

void TrsmProblem::merge(std::vector<Problem*> subproblems) {
    return;
}

