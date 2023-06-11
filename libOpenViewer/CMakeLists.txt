# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 - Present Romain Augier
# All rights reserved.

add_subdirectory(src)

if(RUN_TESTS EQUAL 1)
    message(STATUS "Building libOpenViewer tests")
    add_subdirectory(tests)
endif()