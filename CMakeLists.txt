# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 - Present Romain Augier
# All rights reserved.

# Global #

cmake_minimum_required(VERSION 3.15)

# Project
project(OpenViewer) 

# CPP
set(CMAKE_CXX_STANDARD 17)

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