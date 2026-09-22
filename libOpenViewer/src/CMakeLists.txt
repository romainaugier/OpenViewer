# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 - Present Romain Augier
# All rights reserved.

# libOpenViewer library

include(target_options)
include(GNUInstallDirs)

# Explicit, not GLOB. A glob silently pulls in whatever is lying around in the
# tree, which is how the attic/ files kept breaking the build after the data
# model changed, and it does not re-run when a file is added.
set(OPENVIEWER_SOURCES
    image_reader.cpp
    log.cpp
    media.cpp
    media_cache.cpp
    readers/reader_exr.cpp
    readers/reader_stb.cpp
    readers/reader_tiff.cpp
)

add_library(${OPENVIEWER_LIBS} SHARED ${OPENVIEWER_SOURCES})

set_target_properties(${OPENVIEWER_LIBS} PROPERTIES PREFIX ""
                                                    CXX_STANDARD 17
                                                    CXX_STANDARD_REQUIRED ON
                                                    DEBUG_POSTFIX "d"
                                                    LINKER_LANGUAGE CXX
                                                    EXPORT_LINK_INTERFACE_LANGUAGES CXX)

set_target_options(${OPENVIEWER_LIBS})

target_compile_definitions(${OPENVIEWER_LIBS} PRIVATE "-DLOV_BUILD_SHARED")

# src/ is on the private include path so the readers can see readers.hpp, which
# is not part of the installed API.
target_include_directories(${OPENVIEWER_LIBS} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${OPENVIEWER_LIBS} PUBLIC stdromano::stdromano)
target_link_libraries(${OPENVIEWER_LIBS} PUBLIC OpenEXR::OpenEXR)
target_link_libraries(${OPENVIEWER_LIBS} PUBLIC Imath::Imath)
target_link_libraries(${OPENVIEWER_LIBS} PUBLIC TIFF::TIFF)
target_link_libraries(${OPENVIEWER_LIBS} PUBLIC OpenColorIO::OpenColorIO)

# Install

install(TARGETS ${OPENVIEWER_LIBS}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../include/OpenViewer"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

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
