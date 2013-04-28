#!/bin/bash

export CILK_NWORKERS=32
export MKL_NUM_THREADS=1

echo -e "\e[01;34mcompiling...\e[0m"

FLAGS="-O3 -mkl -ipo -xHOST -no-prec-div -fno-strict-aliasing -fno-omit-frame-pointer"
FRAMEWORK="framework.cpp Task.cpp Problem.cpp"

if [ "$1" = "quicksort" ]; then
    icc $FLAGS -o harness quicksort_harness.cpp QuicksortProblem.cpp $FRAMEWORK
    echo -e "\e[0;32mrunning quicksort...\e[0m"
    ./harness

elif [ "$1" = "mergesort" ]; then
    icc $FLAGS -o harness mergesort_harness.cpp MergesortProblem.cpp $FRAMEWORK
    echo -e "\e[0;32mrunning mergesort...\e[0m"
    ./harness

elif [ "$1" = "carma" ]; then
    icc $FLAGS -o harness  carma_harness.cpp CarmaProblem.cpp $FRAMEWORK
    echo -e "\e[0;32mrunning CARMA...\e[0m"
    ./harness 64 262144 64

elif [ "$1" = "strassen" ]; then
    icc $FLAGS -o harness strassen_harness.cpp StrassenProblem.cpp $FRAMEWORK
    echo -e "\e[0;32mrunning STRASSEN...\e[0m"
    ./harness 1024 1024 1024

elif [ "$1" = "trsm" ]; then
    icc $FLAGS -o harness trsm_harness.cpp TrsmProblem.cpp MultProblem.cpp $FRAMEWORK
    echo -e "\e[0;32mrunning TRSM...\e[0m"
    ./harness

else
    echo -e "\e[0;31mERROR: Algorithm not found\e[0m"
    exit
fi

rm -rf harness
