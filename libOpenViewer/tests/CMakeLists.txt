# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 - Present Romain Augier
# All rights reserved.

# libOpenViewer tests #

file(GLOB_RECURSE TEST_FILES *.cpp)

foreach(test_file ${TEST_FILES})
    get_filename_component(TESTNAME ${test_file} NAME_WLE)
    message(STATUS "Adding libOpenViewer test : ${TESTNAME}")

    add_executable(${TESTNAME} ${test_file})
    target_link_libraries(${TESTNAME} ${OPENVIEWER_LIBS})

    add_test(${TESTNAME} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTNAME})
endforeach()

# Copy all binaries from conan in the runtime directory to link dynamic libraries
# to test executables
if(WIN32)
    set(CONAN_DLL_DIR ${CMAKE_SOURCE_DIR}/bin)

    file(GLOB_RECURSE CONAN_DLLS ${CONAN_DLL_DIR}/*.dll)

    list(FILTER CONAN_DLLS EXCLUDE REGEX ".*(libOpenViewer.*|OpenViewer\\.exe)")

    foreach(dll_file ${CONAN_DLLS})
        add_custom_command(
            TARGET ${TESTNAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${dll_file} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        )
    endforeach(dll_file)
endif()