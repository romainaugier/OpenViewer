# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 - Present Romain Augier
# All rights reserved.

add_subdirectory(src)

if(OPENVIEWER_BUILD_FUZZERS)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR "OPENVIEWER_BUILD_FUZZERS needs Clang (libFuzzer)")
    endif()

    message(STATUS "Building libOpenViewer fuzzers")

    # Coverage for libFuzzer to steer by, and sanitizers where the bugs are
    target_compile_options(${OPENVIEWER_LIBS} PRIVATE -fsanitize=fuzzer-no-link,address,undefined -g)
    target_link_options(${OPENVIEWER_LIBS} PRIVATE -fsanitize=address,undefined)

    add_subdirectory(tests/fuzz)
endif()

if(RUN_TESTS EQUAL 1)
    message(STATUS "Building libOpenViewer tests")
    add_subdirectory(tests)
endif()