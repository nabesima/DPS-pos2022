#!/bin/sh

TARGET="r"

if [ $# -ge 1 ]; then
    TARGET=$1
fi

if [ ${TARGET} = "d" ]; then
    # From cmake-3.13, the following simple instructions are avairable
    # cmake -B Debug -DCMAKE_BUILD_TYPE=Debug
    # cmake --build Debug 
    mkdir -p Debug
    cd Debug
    cmake .. -DCMAKE_BUILD_TYPE=Debug
    cmake --build .
    cd ..
elif [ ${TARGET} = "r" ]; then
    # From cmake-3.13, the following simple instructions are avairable
    # cmake -B Release -DCMAKE_BUILD_TYPE=Release
    # cmake --build Release 
    mkdir -p Release
    cd Release
    cmake .. -DCMAKE_BUILD_TYPE=Release
    cmake --build .
    cd ..
elif [ ${TARGET} = "c" ]; then
    if [ -f "Release/Makefile" ]; then
        cd Release
        make clean
        cd ..
    fi
    if [ -f "Debug/Makefile" ]; then
        cd Debug
        make clean
        cd ..
    fi
fi

