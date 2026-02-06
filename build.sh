#!/bin/bash

# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 - Present Romain Augier
# All rights reserved.

BUILDTYPE="Release"
RUNTESTS=0
REMOVEOLDDIR=0
EXPORTCOMPILECOMMANDS=0
VERSION="0.0.0"
INSTALLDIR="$PWD/install"
INSTALL=0
THREADSAN=0
UBSAN=0
ADDRSAN=0
LEAKSAN=0
VCPKG_PATH="$PWD/vcpkg"
VCPKG_USER_DEFINED=0

# Little function to parse command line arguments
parse_args()
{
    [ "$1" == "--debug" ] && BUILDTYPE="Debug"

    [ "$1" == "--reldebug" ] && BUILDTYPE="RelWithDebInfo"

    [ "$1" == "--tests" ] && RUNTESTS=1

    [ "$1" == "--clean" ] && REMOVEOLDDIR=1

    [ "$1" == "--install" ] && INSTALL=1

    [ "$1" == "--threadsan" ] && THREADSAN=1

    [ "$1" == "--ubsan" ] && UBSAN=1

    [ "$1" == "--addrsan" ] && ADDRSAN=1

    [ "$1" == "--leaksan" ] && LEAKSAN=1

    [ "$1" == "--export-compile-commands" ] && EXPORTCOMPILECOMMANDS=1

    [ "$1" == *"version"* ] && parse_version $1

    [ "$1" == *"installdir"* ] && parse_install_dir $1

    [ "$1" == *"vpckgpath"* ] && parse_vcpkg_path $1
}

# Little function to parse the version from a command line argument
parse_version()
{
    VERSION="$( cut -d ':' -f 2- <<< "$s" )"
    log_info "Version specified by user: $VERSION"
}

# Little function to parse the installation dir from a command line argument
parse_install_dir()
{
    INSTALLDIR="$( cut -d ':' -f 2- <<< "$s" )"
    log_info "Install directory specified by user: $INSTALLDIR"
}

# Little function to parse the vcpkg path from a command line argument
parse_vcpkg_path()
{
    VCPKG_PATH="$( cut -d ':' -f 2- <<< "$s" )"
    log_info "Vcpkg path specified by user: $VCPKG_PATH"
}

# Little function to log an information message to the console
log_info()
{
    echo "[INFO] : $1"
}

# Little function to log a warning message to the console
log_warning()
{
    echo "[WARNING] : $1"
}

# Little function to log an error message to the console
log_error()
{
    echo "[ERROR] : $1"
}

# Wrap to avoid command line args propagation
source_vcpkg_bootstrap()
{
    source bootstrap-vcpkg.sh
}

# Entry point

log_info "Building OpenViewer"

for arg in "$@"
do
    parse_args "$arg"
done

if [[ $UBSAN -eq 1 ]]; then
    if [[ $ADDRSAN -eq 1 ]]; then
        log_error "Undefined Behavior Sanitizer and Address Sanitizer are not compatible"
        exit 1
    fi

    if [[ $LEAKSAN -eq 1 ]]; then
        log_error "Undefined Behavior Sanitizer and Leak Sanitizer are not compatible"
        exit 1
    fi

    if [[ $THREADSAN -eq 1 ]]; then
        log_error "Undefined Behavior Sanitizer and Thread Sanitizer are not compatible"
        exit 1
    fi
fi

if [[ ! -d "vcpkg" ]]; then
    if [[ $VCPKG_USER_DEFINED -eq 1 ]]; then
        log_info "Using existing vcpkg installation"
        export VCPKG_ROOT=$VCPKG_PATH
    else
        log_info "Vcpkg can't be found, cloning and preparing it"
        git clone https://github.com/romainaugier/vcpkg.git
        cd vcpkg
        source_vcpkg_bootstrap
        cd ..
        export VCPKG_ROOT=$PWD/vcpkg
    fi
fi

log_info "Vcpkg root: $VCPKG_ROOT"

export CMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake

if [[ $PATH != *$VCPKG_ROOT* ]]; then
    log_info "Can't find vcpkg root in PATH, appending it"
    export PATH=$PATH:$VCPKG_ROOT
fi

# For OpenCL
export ASAN_OPTIONS=protect_shadow_gap=0

log_info "Build type: $BUILDTYPE"
log_info "Build version: $VERSION"

if [[ -d "build" && $REMOVEOLDDIR -eq 1 ]]; then
    log_info "Removing old build directory"
    rm -rf build
fi

if [[ -d "install" && $REMOVEOLDDIR -eq 1 ]]; then
    log_info "Removing old install directory"
    rm -rf install
fi

cmake -S . -B build -DRUN_TESTS=$RUNTESTS \
                    -DCMAKE_EXPORT_COMPILE_COMMANDS=$EXPORTCOMPILECOMMANDS \
                    -DCMAKE_BUILD_TYPE=$BUILDTYPE \
                    -DVERSION=$VERSION \
                    -DTHREADSAN=$THREADSAN \
                    -DUBSAN=$UBSAN \
                    -DADDRSAN=$ADDRSAN \
                    -DLEAKSAN=$LEAKSAN

if [[ $? -ne 0 ]]; then
    log_error "Error during CMake configuration"
    exit 1
fi

cd build

cmake --build . -- -j $(nproc)

if [[ $? -ne 0 ]]; then
    log_error "Error during CMake build"
    cd ..
    exit 1
fi

if [[ $RUNTESTS -eq 1 ]]; then
    ctest --output-on-failure -C $BUILDTYPE

    if [[ $? -ne 0 ]]; then
        log_error "Error during CMake testing"
        cd ..
        exit 1
    fi
fi

if [[ $INSTALL -eq 1 ]]; then
    cmake --install . --config $BUILDTYPE --prefix $INSTALLDIR

    if [[ $? -ne 0 ]]; then
        log_error "Error during CMake installation"
        cd ..
        exit 1
    fi
fi

cd ..

if [[ $EXPORTCOMPILECOMMANDS -eq 1 ]]; then
    cp ./build/compile_commands.json ./compile_commands.json
    log_info "Copied compile_commands.json to root directory"
fi
