# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 - Present Romain Augier
# All rights reserved.

# libOpenViewerUtils library #

set(LIBNAME libOpenViewerUtils)

file(GLOB_RECURSE src_files *.cpp)

add_definitions(-DLOVU_BUILD_DLL -DSPDLOG_FMT_EXTERNAL)

add_library(${LIBNAME} SHARED ${src_files} ${h_files})
target_link_libraries(${LIBNAME} ${CONAN_LIBS})

add_custom_command(
    TARGET ${LIBNAME}
    POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:${LIBNAME}> ${CMAKE_SOURCE_DIR}/bin
)