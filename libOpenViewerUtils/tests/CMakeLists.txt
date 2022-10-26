# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 - Present Romain Augier
# All rights reserved.

# libOpenViewerUtils tests #

# Filesystem
set(TESTNAME TestFilesystem)

add_executable(${TESTNAME} test_filesystem.cpp)
target_link_libraries(${TESTNAME} ${OPENVIEWERUTILS_LIBS})

add_test(${TESTNAME} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTNAME})

# Bitarray
set(TESTNAME TestBitArray)

add_executable(${TESTNAME} test_bitarray.cpp)
target_link_libraries(${TESTNAME} ${OPENVIEWERUTILS_LIBS})

add_test(${TESTNAME} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTNAME})