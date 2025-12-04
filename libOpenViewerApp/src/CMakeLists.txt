# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 - Present Romain Augier
# All rights reserved.

# libOpenViewer library

include(target_options)
include(GNUInstallDirs)

file(GLOB_RECURSE SRC_FILES *.cpp)

add_library(${OPENVIEWERAPP_LIBS} SHARED ${SRC_FILES})

set_target_properties(${OPENVIEWERAPP_LIBS} PROPERTIES PREFIX ""
                                                       CXX_STANDARD 17
                                                       DEBUG_POSTFIX "d"
                                                       LINKER_LANGUAGE CXX
                                                       EXPORT_LINK_INTERFACE_LANGUAGES CXX)

target_include_directories(${OPENVIEWERAPP_LIBS} PRIVATE ${imgui_INCLUDE_DIRS})

set_target_options(${OPENVIEWERAPP_LIBS})

target_link_libraries(${OPENVIEWERAPP_LIBS} PUBLIC ${OPENVIEWER_LIBS})
target_link_libraries(${OPENVIEWERAPP_LIBS} PUBLIC imgui::imgui)

# Install

install(CODE [[
    file(GET_RUNTIME_DEPENDENCIES
         EXECUTABLES $<TARGET_FILE:libOpenViewerApp>
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
