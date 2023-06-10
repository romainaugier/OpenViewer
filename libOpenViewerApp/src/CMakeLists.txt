# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 - Present Romain Augier
# All rights reserved.

# libOpenViewer library #

file(GLOB_RECURSE src_files *.cpp)

add_definitions(-DLOVA_BUILD_EXPORT)

add_library(${OPENVIEWERAPP_LIBS} SHARED ${src_files})
set_target_properties(${OPENVIEWERAPP_LIBS} PROPERTIES PREFIX "")
target_link_libraries(${OPENVIEWERAPP_LIBS} ${CONAN_LIBS} ${OPENVIEWER_LIBS} ${IMGUI_LIB})

add_custom_command(
    TARGET ${OPENVIEWERAPP_LIBS}
    POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:${OPENVIEWERAPP_LIBS}> ${CMAKE_SOURCE_DIR}/bin
)