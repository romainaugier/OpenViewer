# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 - Present Romain Augier
# All rights reserved.

# libOpenViewer tests
#
# One executable per file, all sharing lov_test.hpp. Listed explicitly so that
# adding a test is a deliberate act and a stray .cpp does not become a test.

include(target_options)

set(OPENVIEWER_TESTS
    test_media_info.cpp
    test_image_reader.cpp
    test_exr_read.cpp
    test_media.cpp
    test_media_cache.cpp
    test_log.cpp
    test_log_compiled_out.cpp
    test_image_fuzz.cpp
)

foreach(test_file ${OPENVIEWER_TESTS})
    get_filename_component(TESTNAME ${test_file} NAME_WLE)

    message(STATUS "Adding libOpenViewer test : ${TESTNAME}")

    add_executable(${TESTNAME} ${test_file})

    set_target_properties(${TESTNAME} PROPERTIES CXX_STANDARD 17
                                                 CXX_STANDARD_REQUIRED ON)

    set_target_options(${TESTNAME})

    # The harness and the synthetic image generator live next to the tests.
    target_include_directories(${TESTNAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

    target_link_libraries(${TESTNAME} PRIVATE ${OPENVIEWER_LIBS})

    # stdromano.dll comes from its own install tree: copy every runtime DLL the
    # test depends on next to it. vcpkg already does this for its own packages.
    if(WIN32)
        add_custom_command(TARGET ${TESTNAME} POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy_if_different
                                   $<TARGET_RUNTIME_DLLS:${TESTNAME}>
                                   $<TARGET_FILE_DIR:${TESTNAME}>
                           COMMAND_EXPAND_LISTS)
    endif()

    add_test(NAME ${TESTNAME}
             COMMAND ${TESTNAME}
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

    # A hang must fail the run, not block it until the CI job limit
    set_tests_properties(${TESTNAME} PROPERTIES TIMEOUT 120)
endforeach()
