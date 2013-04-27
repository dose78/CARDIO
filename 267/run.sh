#!/bin/bash

export CILK_NWORKERS=32
export MKL_NUM_THREADS=1

echo -e "\e[01;34mcompiling...\e[0m"

FLAGS="-O3 -mkl -ipo -xHOST -no-prec-div -fno-strict-aliasing -fno-omit-frame-pointer"

if [ "$1" = "quicksort" ]; then
    icc $FLAGS -o harness quicksort_harness.cpp QuicksortProblem.cpp framework.cpp
    echo -e "\e[0;32mrunning quicksort...\e[0m"
    ./harness

elif [ "$1" = "mergesort" ]; then
    icc $FLAGS -o harness mergesort_harness.cpp MergesortProblem.cpp framework.cpp
    echo -e "\e[0;32mrunning mergesort...\e[0m"
    ./harness

elif [ "$1" = "carma" ]; then
    icc $FLAGS -o harness  carma_harness.cpp CarmaProblem.cpp framework.cpp
    echo -e "\e[0;32mrunning CARMA...\e[0m"
    ./harness 64 262144 64

elif [ "$1" = "strassen" ] || [ "$1" = "strassen.c" ]; then
    icc -mkl -o harness -O3 -ipo -xHOST -no-prec-div -fno-strict-aliasing -fno-omit-frame-pointer test_strassen.cpp framework.cpp StrassenProblem.cpp
    #icc -mkl -o harness  -O3 test_strassen.cpp framework.cpp StrassenProblem.cpp
    echo -e "\e[0;32mrunning STRASSEN...\e[0m"
    ./harness 

elif [ "$1" = "trsm" ] || [ "$1" = "trsm.c" ]; then
    icc -mkl -o trsm -O3 -ipo -xHOST -no-prec-div -fno-strict-aliasing -fno-omit-frame-pointer TrsmProblem.cpp MultProblem.cpp framework.cpp test.cpp
    echo -e "\e[0;32mrunning TRSM...\e[0m"
    ./trsm

else
    echo -e "\e[0;31mERROR: Algorithm not found\e[0m"
    exit
fi

rm -rf harness
rm -rf trsm
