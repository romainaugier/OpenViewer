# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 - Present Romain Augier
# All rights reserved.

# Global #

# 3.21 for $<TARGET_RUNTIME_DLLS>, used to put dependent DLLs next to the tests
cmake_minimum_required(VERSION 3.21)

# Project
project(OpenViewer)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Target architecture, same detection as stdromano. CMAKE_OSX_ARCHITECTURES
# wins when cross building on apple (e.g. an x86_64 build from an arm64 mac).
if(CMAKE_OSX_ARCHITECTURES)
    list(GET CMAKE_OSX_ARCHITECTURES 0 OPENVIEWER_TARGET_ARCH)
else()
    set(OPENVIEWER_TARGET_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
endif()

string(TOLOWER "${OPENVIEWER_TARGET_ARCH}" OPENVIEWER_TARGET_ARCH)

if(OPENVIEWER_TARGET_ARCH MATCHES "^(x86_64|amd64|x64)$")
    set(OPENVIEWER_ARCH_X86_64 ON)
elseif(OPENVIEWER_TARGET_ARCH MATCHES "^(aarch64|arm64|armv8.*)$")
    set(OPENVIEWER_ARCH_AARCH64 ON)
else()
    message(FATAL_ERROR "Unsupported target architecture \"${OPENVIEWER_TARGET_ARCH}\", "
                        "OpenViewer supports x86_64 and aarch64")
endif()

message(STATUS "OpenViewer target: ${CMAKE_SYSTEM_NAME} ${OPENVIEWER_TARGET_ARCH}")

# Nothing in libOpenViewer uses OpenCL today. Off by default: on macOS it is a
# deprecated framework, and on Windows the fallback below rewrites vcpkg.json
# at configure time, which should not happen to a tracked file by accident.
option(OPENVIEWER_ENABLE_OPENCL "Look for OpenCL (unused by libOpenViewer for now)" OFF)
option(OPENVIEWER_BUILD_FUZZERS "Builds the libFuzzer executables in libOpenViewer/tests/fuzz (Clang only)" OFF)

# Every binary and shared library in one place. On Windows an executable only
# finds DLLs in its own directory or on PATH, so without this the tests cannot
# load libOpenViewer.dll. Harmless elsewhere, where the build tree RPATH works.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Set libraries names
set(OPENVIEWER_LIBS libOpenViewer)
set(OPENVIEWERAPP_LIBS libOpenViewerApp)
set(OPENVIEWER_APP OpenViewer)

if(RUN_TESTS EQUAL 1)
    message(STATUS "RUN_TESTS defined, building and running tests")
    enable_testing()

    set(TEST_DATA_DIR ${CMAKE_SOURCE_DIR}/test)

    add_definitions(-DTEST_DATA_DIR="${TEST_DATA_DIR}")
else()
    add_definitions(-DTEST_DATA_DIR="")
endif()

# OpenCL (for stdromano and OpenViewer)
if(NOT OPENVIEWER_ENABLE_OPENCL)
    message(STATUS "OpenCL disabled (OPENVIEWER_ENABLE_OPENCL=OFF)")
elseif(UNIX)
    find_package(PkgConfig QUIET)

    if(PkgConfig_FOUND)
        pkg_check_modules(OPENCL_PKG QUIET OpenCL)
    endif()

    find_library(SYSTEM_OPENCL_LIB
        NAMES OpenCL OpenCL.so OpenCL.so.1
        HINTS /usr/lib64 /lib64 /usr/lib /usr/local/lib
    )

    find_path(SYSTEM_OPENCL_INCLUDE_DIR
        NAMES CL/cl.h
        HINTS /usr/include /usr/local/include /usr/include/CL
    )

    if(SYSTEM_OPENCL_LIB AND SYSTEM_OPENCL_INCLUDE_DIR)
        message(STATUS "Found system OpenCL: ${SYSTEM_OPENCL_LIB}, includes: ${SYSTEM_OPENCL_INCLUDE_DIR}")

        if(TARGET OpenCL::OpenCL)
            message(STATUS "Overriding existing OpenCL::OpenCL imported target to system OpenCL")

            set_target_properties(OpenCL::OpenCL PROPERTIES
                IMPORTED_LOCATION "${SYSTEM_OPENCL_LIB}"
                INTERFACE_INCLUDE_DIRECTORIES "${SYSTEM_OPENCL_INCLUDE_DIR}"
            )
        else()
            add_library(OpenCL::OpenCL UNKNOWN IMPORTED GLOBAL)

            set_target_properties(OpenCL::OpenCL PROPERTIES
                IMPORTED_LOCATION "${SYSTEM_OPENCL_LIB}"
                INTERFACE_INCLUDE_DIRECTORIES "${SYSTEM_OPENCL_INCLUDE_DIR}"
            )
        endif()

        set(OpenCL_INCLUDE_DIR ${SYSTEM_OPENCL_INCLUDE_DIR})
    else()
        message(STATUS "System OpenCL not found via find_library/find_path; will try find_package or vcpkg fallback")

        find_package(OpenCL QUIET)

        if(NOT TARGET OpenCL::OpenCL AND OpenCL_LIBRARY)
            add_library(OpenCL::OpenCL UNKNOWN IMPORTED GLOBAL)
            set_target_properties(OpenCL::OpenCL PROPERTIES
                IMPORTED_LOCATION "${OpenCL_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${OpenCL_INCLUDE_DIR}"
            )
        endif()
    endif()

elseif(WIN32)
    # Usually OpenCL comes with CUDA on windows (this is how I install it) so we ask for Cuda toolkit and
    # then use the OpenCL provided by Nvidia
    find_package(CUDAToolkit)

    if(NOT ${CUDAToolkit_FOUND})
        message(STATUS "Cannot find OpenCL via Cuda Toolkit, downloading it using vcpkg")

        file(READ ${CMAKE_SOURCE_DIR}/vcpkg.json JSON_CONTENT)

        string(REGEX REPLACE
               "(\"dependencies\"[ \t]*:[ \t]*\\[[^]]*)(\\])"
               "\\1, \"opencl\"\\2"
               JSON_CONTENT
               "${JSON_CONTENT}")

        file(WRITE ${CMAKE_SOURCE_DIR}/vcpkg.json "${JSON_CONTENT}")

        execute_process(
            COMMAND vcpkg install
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/vcpkg
        )
        set(OpenCL_ROOT "${CMAKE_SOURCE_DIR}/vcpkg/packages/opencl_x64-windows")
    else()
        set(OpenCL_ROOT "${CUDA_TOOLKIT_ROOT_DIR}")
    endif()

    find_package(OpenCL REQUIRED)
endif()

# Python
find_package(Python 3.11 COMPONENTS Interpreter Development REQUIRED)
include_directories(${Python_INCLUDE_DIRS})

# Nanobind
add_subdirectory(ext/nanobind)

# stdromano
# The stdromano install also carries the spdlog and concurrentqueue headers it
# vendors, so this include dir is where <spdlog/spdlog.h> comes from. Pulling a
# second spdlog in through vcpkg would give two copies that disagree on the
# bundled fmt.
set(stdromano_DIR "${CMAKE_SOURCE_DIR}/ext/stdromano/install/cmake")
find_package(stdromano CONFIG REQUIRED)
include_directories(${stdromano_INCLUDE_DIR})

# STB
include_directories(stb/include)

# OpenEXR
find_package(OpenEXR CONFIG REQUIRED)

# OpenColorIO
find_package(OpenColorIO CONFIG REQUIRED)

# TIFF
find_package(TIFF REQUIRED)

# Library OpenViewer
include_directories(libOpenViewer/include)
add_subdirectory(libOpenViewer)

# Library OpenViewerApp
# TODO(W6): the directory is empty, so configuring it fails. It comes back when
# there is a display layer to put in it.
# include_directories(libOpenViewerApp/include)
# add_subdirectory(libOpenViewerApp)

# OpenViewer App
add_subdirectory(app)
