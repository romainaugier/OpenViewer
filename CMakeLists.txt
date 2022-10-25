# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 - Present Romain Augier
# All rights reserved.

# Global #

cmake_minimum_required(VERSION 3.15)

# Project
project(OpenViewer) 

# CPP
set(CMAKE_CXX_STANDARD 17)

include(${CMAKE_SOURCE_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

# Library OpenViewerUtils
include_directories(libOpenViewerUtils/include)
add_subdirectory(libOpenViewerUtils)

# Library OpenViewer
# include_directories(libOpenViewer/include)
# add_subdirectory(libOpenViewer)