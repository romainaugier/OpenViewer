# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 - Present Romain Augier
# All rights reserved.

# OpenViewer Application #

include(target_options)

add_executable(${OPENVIEWER_APP} openviewer.cpp)

set_target_properties(${OPENVIEWER_APP} PROPERTIES CXX_STANDARD 17
                                                   CXX_STANDARD_REQUIRED ON)

# Same flags as the library, sanitizers included: an executable linking an
# ASan-instrumented libOpenViewer without the ASan runtime refuses to start.
set_target_options(${OPENVIEWER_APP})

# ${CONAN_LIBS} was left over from the Conan build and expands to nothing;
# ${OPENVIEWERAPP_LIBS} names a target that does not exist yet. The app links
# the library directly until there is a libOpenViewerApp to link.
target_link_libraries(${OPENVIEWER_APP} PRIVATE ${OPENVIEWER_LIBS})
