# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 - Present Romain Augier
# All rights reserved.

# libOpenViewer library #

file(GLOB_RECURSE src_files *.cpp)

list(FILTER src_files EXCLUDE REGEX ".*python.cpp")

add_definitions(-DLOV_BUILD_EXPORT -DSPDLOG_FMT_EXTERNAL)

add_library(${OPENVIEWER_LIBS} SHARED ${src_files} ${h_files})
set_target_properties(${OPENVIEWER_LIBS} PROPERTIES PREFIX "")
target_link_libraries(${OPENVIEWER_LIBS} ${CONAN_LIBS} ${Python_LIBRARIES} ${OPENVIEWERUTILS_LIBS})

add_custom_command(
    TARGET ${OPENVIEWER_LIBS}
    POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:${OPENVIEWER_LIBS}> ${OUTPUT_BIN_DIR}
)

set(PYOPENVIEWER_MODULE PyOpenViewer)

pybind11_add_module(${PYOPENVIEWER_MODULE} python.cpp)
target_link_libraries(${PYOPENVIEWER_MODULE} PUBLIC ${OPENVIEWER_LIBS})

add_custom_command(
    TARGET ${PYOPENVIEWER_MODULE}
    POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:${PYOPENVIEWER_MODULE}> ${OUTPUT_BIN_DIR}
)