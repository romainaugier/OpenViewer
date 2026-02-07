# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 - Present Romain Augier
# All rights reserved.

# OpenViewer Application #

add_executable(${OPENVIEWER_APP} openviewer.cpp)
target_link_libraries(${OPENVIEWER_APP} ${CONAN_LIBS} ${OPENVIEWERAPP_LIBS})
