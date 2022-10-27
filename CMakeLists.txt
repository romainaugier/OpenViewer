# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 - Present Romain Augier
# All rights reserved.

# Global #

cmake_minimum_required(VERSION 3.15)

# Project
project(OpenViewer) 

# CPP
set(CMAKE_CXX_STANDARD 17)

# Compilation flags
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	set(MSVC_COMPILER_FLAGS "/fp:fast /Os /Oy /GT /GL /Oi /Zi /Gm- /Zc:inline /Qpar")

	if(DEFINED AVX2)
		add_definitions("-DOV_VECTOR_EXT=\"avx2\"")
		message(STATUS "Building with AVX2")
		set(MSVC_COMPILER_FLAGS "${MSVC_COMPILER_FLAGS} /arch:AVX2")
    elseif(DEFINED AVX)
		add_definitions("-DOV_VECTOR_EXT=\"avx\"")
		message(STATUS "Building with AVX")
		set(MSVC_COMPILER_FLAGS "${MSVC_COMPILER_FLAGS} /arch:AVX")
    elseif(DEFINED SSE2)
		add_definitions("-DOV_VECTOR_EXT=\"sse2\"")
		message(STATUS "Building with SSE2")
		set(MSVC_COMPILER_FLAGS "${MSVC_COMPILER_FLAGS} /arch:SSE2")
    else()
		add_definitions("-DOV_VECTOR_EXT=\"avx2\"")
		message(STATUS "Building with AVX2")
		set(MSVC_COMPILER_FLAGS "${MSVC_COMPILER_FLAGS} /arch:AVX2")
	endif()

	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}" "${MSVC_COMPILER_FLAGS} /O2")
	add_link_options("/LTCG")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
	set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} -O3")
	add_compile_options(-fopenmp -mveclibabi=svml -mf16c -fpermissive -Wall)
	add_link_options(-lstdc++fs)
else()
	message(FATAL_ERROR "Your compiler is not supported yet, CMake will exit.")
endif()

# # Windows
# SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Set libraries names
set(OPENVIEWERUTILS_LIBS libOpenViewerUtils)
set(OPENVIEWER_LIBS libOpenViewer)

if(DEFINED RUN_TESTS)
    message(STATUS "RUN_TESTS defined, building and running tests")
    enable_testing()

    set(TEST_DATA_DIR ${CMAKE_SOURCE_DIR}/test)

    add_definitions(-DTEST_DATA_DIR="${TEST_DATA_DIR}")
endif()

include(${CMAKE_SOURCE_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

# Library OpenViewerUtils
include_directories(libOpenViewerUtils/include)
add_subdirectory(libOpenViewerUtils)

# Library OpenViewer
include_directories(libOpenViewer/include)
add_subdirectory(libOpenViewer)