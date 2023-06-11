#!/bin/bash

echo Building OpenViewer

BUILDTYPE="RelWithDebInfo"
RUNTESTS=0
REMOVEOLDDIR=0
EXPORTCOMPILECOMMANDS=0
SANITIZE=0

parse_args()
{
    [ "$1" == "--debug" ] && BUILDTYPE="Debug"
    [ "$1" == "--tests" ] && RUNTESTS=1
    [ "$1" == "--clean" ] && REMOVEOLDDIR=1
    [ "$1" == "--export-compile-commands" ] && EXPORTCOMPILECOMMANDS=1
    [ "$1" == "--sanitize" ] && SANITIZE=1
}

for arg in "$@"
do
    parse_args "$arg"
done

if [[ -d "build" && $REMOVEOLDDIR -eq 1 ]]
then
    rm -rf build
fi

cmake -S . -B build -DSANITIZE=$SANITIZE -DRUN_TESTS=$RUNTESTS -DCMAKE_EXPORT_COMPILE_COMMANDS=$EXPORTCOMPILECOMMANDS -DCMAKE_BUILD_TYPE=$BUILDTYPE

cd build

cmake --build . -- -j $(nproc)

if [[ $RUNTESTS -eq 1 ]]
then
    ctest --output-on-failure
fi

cd ..

if [[ $EXPORTCOMPILECOMMANDS -eq 1 ]]
then
    cp ./build/compile_commands.json ./compile_commands.json
fi