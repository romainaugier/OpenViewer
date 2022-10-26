# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 - Present Romain Augier
# All rights reserved.

add_subdirectory(src)

if(DEFINED RUN_TESTS)
    message(STATUS "Building libOpenViewerUtils tests")
    add_subdirectory(tests)
endif()