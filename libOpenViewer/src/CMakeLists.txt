# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 - Present Romain Augier
# All rights reserved.

# libOpenViewer library

include(target_options)
include(GNUInstallDirs)

file(GLOB_RECURSE SRC_FILES *.cpp)

list(FILTER SRC_FILES EXCLUDE REGEX ".*python.cpp")

add_library(${OPENVIEWER_LIBS} SHARED ${SRC_FILES})

set_target_properties(${OPENVIEWER_LIBS} PROPERTIES PREFIX ""
                                                    CXX_STANDARD 17
                                                    DEBUG_POSTFIX "d"
                                                    LINKER_LANGUAGE CXX
                                                    EXPORT_LINK_INTERFACE_LANGUAGES CXX)

set_target_options(${OPENVIEWER_LIBS})

target_compile_definitions(${OPENVIEWER_LIBS} PRIVATE "-DLOV_BUILD_SHARED")

target_link_libraries(${OPENVIEWER_LIBS} PUBLIC ${Python_LIBRARIES})
target_link_libraries(${OPENVIEWER_LIBS} PUBLIC stdromano::stdromano)
target_link_libraries(${OPENVIEWER_LIBS} PUBLIC OpenEXR::OpenEXR)
target_link_libraries(${OPENVIEWER_LIBS} PUBLIC OpenColorIO::OpenColorIO)

# Install

install(CODE [[
    file(GET_RUNTIME_DEPENDENCIES
         EXECUTABLES $<TARGET_FILE:libOpenViewer>
         RESOLVED_DEPENDENCIES_VAR RESOLVED_DEPS
         UNRESOLVED_DEPENDENCIES_VAR UNRESOLVED_DEPS
         PRE_EXCLUDE_REGEXES "api-ms-" "ext-ms-" "hvsifiletrust" "pdmutilities" "aclui"
         POST_EXCLUDE_REGEXES ".*[Ss]ystem32/.*\\.dll")

    foreach(FILE ${RESOLVED_DEPS})
        file(INSTALL
             DESTINATION "${CMAKE_INSTALL_PREFIX}/bin"
             TYPE SHARED_LIBRARY
             FOLLOW_SYMLINK_CHAIN
             FILES "${FILE}")

    endforeach()
]])
