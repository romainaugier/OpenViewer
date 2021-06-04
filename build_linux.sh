#!/bin/bash
# check if directory exists #

if [ -d "build" ]; then
    rm -r build
fi

mkdir build
cmake -S ./ -B build
cd build
make
