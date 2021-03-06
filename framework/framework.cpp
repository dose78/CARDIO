#include "framework.h"
#include "debug.h"

std::string Framework::interleaving; // Determines depth and interleaving

bool Framework::shouldRunBaseCase(int depth) {
    return depth >= interleaving.size();
}

void Framework::solveTask(Task* task, int depth) {
    std::vector<Problem*> problems = task->getProblems();
    for(std::vector<Problem*>::iterator problemIter = problems.begin(); problemIter != problems.end(); problemIter++) {
        Problem *subproblem = *problemIter;
        solve(subproblem, depth);
    }
}

void Framework::deleteSubproblems(std::vector<Problem*> subproblems) {
    for(std::vector<Problem*>::iterator problemIter = subproblems.begin(); problemIter != subproblems.end(); problemIter++) {
        Problem *subproblem = *problemIter;
        delete subproblem;
    }
}

std::vector<Problem*> Framework::getSubproblemsFromTasks(std::vector<Task*> tasks) {
    std::vector<Problem*> subproblems;
    for(std::vector<Task*>::iterator taskIterator = tasks.begin(); taskIterator != tasks.end(); taskIterator++) {
        Task *task = *taskIterator;
        std::vector<Problem*> newSubproblems = task->getProblems();
        // subproblems.reserve(subproblems.size() + newSubproblems.size());
        subproblems.insert(subproblems.end(), newSubproblems.begin(), newSubproblems.end());
        delete task;
    }
    return subproblems;
}

void Framework::setNumBs(std::vector<Problem*> problems, int numBs) {
    for(std::vector<Problem*>::iterator problemIter = problems.begin(); problemIter != problems.end(); problemIter++) {
        Problem *problem = *problemIter;
        problem->numBs = numBs;
    }
}

void Framework::setNumBs(std::vector<Task*> tasks, int numBs) {
    for(std::vector<Task*>::iterator taskIterator = tasks.begin(); taskIterator != tasks.end(); taskIterator++) {
        Task *task = *taskIterator;
        setNumBs(task->getProblems(), numBs);
    }
}

void Framework::solve(Problem* problem, int depth) {
    if (problem->canRunBaseCase() && (problem->mustRunBaseCase() || shouldRunBaseCase(depth))) {
        problem->runBaseCase();
        return;
    }

    std::vector<Problem*> subproblems;
    if (interleaving[depth] == 'B') {
        std::vector<Task*> tasks = problem->split();
        setNumBs(tasks, problem->numBs+1);
        for(std::vector<Task*>::iterator taskIterator = tasks.begin(); taskIterator != tasks.end(); taskIterator++) {
            Task *task = *taskIterator;
            cilk_spawn solveTask(task, depth+1);
        }
        cilk_sync;
        subproblems = getSubproblemsFromTasks(tasks);
        problem->merge(subproblems);
    } else {
        subproblems = problem->splitSequential();
        setNumBs(subproblems, problem->numBs);
        for(std::vector<Problem*>::iterator problemIter = subproblems.begin(); problemIter != subproblems.end(); problemIter++) {
            Problem *subproblem = *problemIter;
            solve(subproblem, depth + 1);
        }
        problem->mergeSequential(subproblems);
    }
    deleteSubproblems(subproblems);

    // Memory Tracking
    if (depth == 0) {
        #ifdef DEBUG
            #ifndef TERSE
                Memory::printAll();
            #endif
        #endif
    }
}

void Framework::solve(Problem* problem, std::string interleaving) {
    problem->numBs = 0;
    Framework::interleaving = interleaving;
    Memory::reset();
    solve(problem, 0);
}
