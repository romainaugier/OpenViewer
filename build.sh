#!/bin/bash

echo Building OpenViewer

BUILDTYPE="Release"
RUNTESTS=0
REMOVEOLDDIR=0
EXPORTCOMPILECOMMANDS=0

parse_args()
{
    [ "$1" == "--debug" ] && BUILDTYPE="Debug"
    [ "$1" == "--tests" ] && RUNTESTS=1
    [ "$1" == "--clean" ] && REMOVEOLDDIR=1
    [ "$1" == "--export-compile-commands" ] && EXPORTCOMPILECOMMANDS=1
}

for arg in "$@"
do
    parse_args "$arg"
done

if [[ -d "build" && $REMOVEOLDDIR -eq 1 ]]
then
    rm -rf build
fi

cmake -S . -B build -DRUN_TESTS=$RUNTESTS -DCMAKE_EXPORT_COMPILE_COMMANDS=$EXPORTCOMPILECOMMANDS 

cd build

cmake --build . --config "$BUILDTYPE" 

if [[ $RUNTESTS -eq 1 ]]
then
    ctest --output-on-failure
fi

cd ..
