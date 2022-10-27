// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/openviewer.h"
#include "OpenViewer/settings.h"

int main(int argc, char** argv)
{
    SET_SPDLOG_FMT;
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Starting Settings Test");

    try
    {
        lov::settings.load();

        lov::settings["autodetect_sequences"] = true;

        const std::string openexr_setting_name = "openexr_thread_count";

        uint8_t openexr_thread_count = lov::settings.get<uint8_t>(openexr_setting_name);

        DEBUG_VAR(openexr_thread_count);

        lov::settings.save();
    }
    catch(const std::exception& err)
    {
        spdlog::error("Test Failed, catched exception : \n  {}", err.what());
        
        std::exit(EXIT_FAILURE);
    }

    spdlog::info("Test Passed");
    return EXIT_SUCCESS;
}