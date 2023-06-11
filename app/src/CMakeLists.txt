# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 - Present Romain Augier
# All rights reserved.

# OpenViewer Application #

add_executable(${OPENVIEWER_APP} openviewer.cpp)
target_link_libraries(${OPENVIEWER_APP} ${CONAN_LIBS} ${OPENVIEWERAPP_LIBS})

add_custom_command(
    TARGET ${OPENVIEWER_APP}
    POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:${OPENVIEWER_APP}> ${OUTPUT_BIN_DIR}
)