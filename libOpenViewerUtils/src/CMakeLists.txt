# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 - Present Romain Augier
# All rights reserved.

# libOpenViewerUtils library #

file(GLOB_RECURSE src_files *.cpp)

add_definitions(-DLOVU_BUILD_EXPORT -DSPDLOG_FMT_EXTERNAL)

add_library(${OPENVIEWERUTILS_LIBS} SHARED ${src_files} ${h_files})
target_link_libraries(${OPENVIEWERUTILS_LIBS} ${CONAN_LIBS})

add_custom_command(
    TARGET ${OPENVIEWERUTILS_LIBS}
    POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:${OPENVIEWERUTILS_LIBS}> ${CMAKE_SOURCE_DIR}/bin
)