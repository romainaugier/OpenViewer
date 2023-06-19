# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 - Present Romain Augier
# All rights reserved.

# Global #

cmake_minimum_required(VERSION 3.15)

# Project
project(OpenViewer) 

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# CPP
set(CMAKE_CXX_STANDARD 17)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

# Compilation flags
if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(CLANG_FLAGS "-Wall -pedantic-errors")

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        if(SANITIZE EQUAL 1)
            set(CLANG_FLAGS "${CLANG_FLAGS} -fsanitize=address -fsanitize=leak")
        endif()

        set(CMAKE_CXX_FLAGS "${CLANG_FLAGS} -O0")
    else()    
        set(CMAKE_CXX_FLAGS "${CLANG_FLAGS} -O3 -march=native")
    endif()
elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(GCC_FLAGS "-D_FORTIFY_SOURCES=2 -pipe -Wall -pedantic-errors")

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        if(SANITIZE EQUAL 1)
            set(GCC_FLAGS "${GCC_FLAGS} -fsanitize=address -fsanitize=leak")
        endif()

        set(CMAKE_CXX_FLAGS "${GCC_FLAGS} -O0")
    else()
        set(CMAKE_CXX_FLAGS "${GCC_FLAGS} -O3 -ffast-math -march=native -ftree-vectorizer-verbose=2 -mveclibabi=svml ")
    endif()
elseif (CMAKE_C_COMPILER_ID STREQUAL "Intel")
	set(ICC_FLAGS "")
elseif (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    set(MSVC_FLAGS "/wd4710 /wd4577")

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        if(SANITIZE EQUAL 1)
            set(MSVC_FLAGS "${MSVC_FLAGS} /fsanitize=address /fsanitize=leak")
        endif()

        set(CMAKE_CXX_FLAGS "${MSVC_FLAGS} /O0")
    else()
        include(find_avx)
        set(CMAKE_CXX_FLAGS "${MSVC_FLAGS} /O2 /GF /Ot /Oy /GT /GL /Oi ${AVX_FLAGS} /Zi /Gm- /Zc:inline /Qpar")
    endif()

	add_link_options("/LTCG")
endif()


# Set libraries names
set(OPENVIEWERUTILS_LIBS libOpenViewerUtils)
set(OPENVIEWER_LIBS libOpenViewer)
set(OPENVIEWER_APP OpenViewer)
set(IMGUI_LIB imgui)
set(OPENVIEWERAPP_LIBS libOpenViewerApp)

if(RUN_TESTS EQUAL 1)
    message(STATUS "RUN_TESTS defined, building and running tests")
    enable_testing()

    set(TEST_DATA_DIR ${CMAKE_SOURCE_DIR}/test)

    add_definitions(-DTEST_DATA_DIR="${TEST_DATA_DIR}")
else()
	add_definitions(-DTEST_DATA_DIR="")
endif()

# Python
find_package(Python 3.11 COMPONENTS Interpreter Development REQUIRED)

include_directories(${Python_INCLUDE_DIRS})

include(${CMAKE_SOURCE_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

# Pybind
add_subdirectory(extern/pybind11)

set(OUTPUT_BIN_DIR "${CMAKE_SOURCE_DIR}/bin")

if(EXISTS ${OUTPUT_BIN_DIR})
    file(REMOVE ${OUTPUT_BIN_DIR})
endif()

file(MAKE_DIRECTORY ${OUTPUT_BIN_DIR})

# STB
include_directories(stb/include)

# Library OpenViewerUtils
include_directories(libOpenViewerUtils/include)
add_subdirectory(libOpenViewerUtils)

# Library OpenViewer
include_directories(libOpenViewer/include)
add_subdirectory(libOpenViewer)

# Imgui
include_directories(imgui/include)
add_subdirectory(imgui)

# Library OpenViewerApp
include_directories(libOpenViewerApp/include)
add_subdirectory(libOpenViewerApp)

# OpenViewer App
add_subdirectory(app)
