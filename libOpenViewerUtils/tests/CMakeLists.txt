# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 - Present Romain Augier
# All rights reserved.

# libOpenViewerUtils tests #

file(GLOB_RECURSE TEST_FILES *.cpp)

foreach(test_file ${TEST_FILES})
    get_filename_component(TESTNAME ${test_file} NAME_WLE)
    message(STATUS "Adding libOpenViewerUtils test : ${TESTNAME}")

    add_executable(${TESTNAME} ${test_file})
    target_link_libraries(${TESTNAME} ${OPENVIEWER_LIBS})

    add_test(${TESTNAME} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTNAME})
endforeach()