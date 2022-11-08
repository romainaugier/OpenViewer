// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/openviewer.h"
#include "OpenViewer/command_line_tools.h"

int main(int argc, char** argv)
{
    SET_SPDLOG_FMT;
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("OpenViewer {} {}", LOV_VERSION_STR, LOV_PLATFORM_STR);

    lov::process_command_line_args(argc, argv);

    return EXIT_SUCCESS;
}