#include <iostream>
#include <vector>
#include <cstdio>

#include <stdlib.h>
// #include <stdio.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <mkl.h>
// #include <cblas.h>

#ifndef PROBLEM
#define PROBLEM

class Problem {
public:
    virtual bool shouldRunBaseCase(int depth) = 0;
    virtual void runBaseCase() = 0;
    virtual std::vector<Problem*> split() = 0;
};

#endif
