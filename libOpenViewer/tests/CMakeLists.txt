# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 - Present Romain Augier
# All rights reserved.

# libOpenViewer tests #

# Generql
set(TESTNAME TestGeneral)

add_executable(${TESTNAME} test_general.cpp)
target_link_libraries(${TESTNAME} ${OPENVIEWER_LIBS})

add_test(${TESTNAME} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTNAME})

# Settings
set(TESTNAME TestSettings)

add_executable(${TESTNAME} test_settings.cpp)
target_link_libraries(${TESTNAME} ${OPENVIEWER_LIBS})

add_test(${TESTNAME} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TESTNAME})

# Copy all binaries from conan in the runtime directory to link dynamic libraries
# to test executables
if(WIN32)
set(CONAN_DLL_DIR ${CMAKE_SOURCE_DIR}/bin)

file(GLOB_RECURSE CONAN_DLLS ${CONAN_DLL_DIR}/*.dll)

foreach(dll_file ${CONAN_DLLS})
    add_custom_command(
        TARGET ${TESTNAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${dll_file} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
endforeach(dll_file)
endif()