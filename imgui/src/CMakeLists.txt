# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 - Present Romain Augier
# All rights reserved.

# imgui library #

file(GLOB_RECURSE src_files *.cpp)

add_library(${IMGUI_LIB} ${src_files})
set_target_properties(${IMGUI_LIB} PROPERTIES PREFIX "")
target_link_libraries(${IMGUI_LIB} ${CONAN_LIBS})

# add_custom_command(
#     TARGET ${IMGUI_LIB}
#     POST_BUILD 
#     COMMAND ${CMAKE_COMMAND} -E copy
#     $<TARGET_FILE:${IMGUI_LIB}> ${CMAKE_SOURCE_DIR}/bin
# )