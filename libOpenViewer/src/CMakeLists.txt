# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 - Present Romain Augier
# All rights reserved.

# libOpenViewer library #

file(GLOB_RECURSE src_files *.cpp)

add_definitions(-DLOV_BUILD_EXPORT -DSPDLOG_FMT_EXTERNAL)

add_library(${OPENVIEWER_LIBS} SHARED ${src_files} ${h_files})
target_link_libraries(${OPENVIEWER_LIBS} ${CONAN_LIBS} ${OPENVIEWERUTILS_LIBS})

add_custom_command(
    TARGET ${OPENVIEWER_LIBS}
    POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:${OPENVIEWER_LIBS}> ${CMAKE_SOURCE_DIR}/bin
)