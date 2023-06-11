# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 - Present Romain Augier
# All rights reserved.

# imgui library #

file(GLOB_RECURSE src_files *.cpp)

add_library(${IMGUI_LIB} ${src_files})
set_target_properties(${IMGUI_LIB} PROPERTIES PREFIX "")
set_property(TARGET ${IMGUI_LIB} PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${IMGUI_LIB} ${CONAN_LIBS})