// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#include "OpenViewer/openviewer.h"

LOV_NAMESPACE_BEGIN

// 
void LOV_DLL process_command_line_args(int argc, char** argv) noexcept;

void LOV_DLL ocio_convert(std::string& filename, 
                          const std::string& ocio_role,
                          const std::string& ocio_display,
                          const std::string& ocio_view,
                          const bool autodetect_sequence = true) noexcept;

void LOV_DLL make_thumbnail(std::string& filename,
                            const uint16_t thumbnail_size) noexcept;

LOV_NAMESPACE_END