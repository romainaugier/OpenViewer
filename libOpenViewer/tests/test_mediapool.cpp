// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/openviewer.h"
#include "OpenViewer/media_pool.h"
#include "OpenViewer/settings.h"
#include "OpenViewerUtils/filesystem.h"
#include "OpenViewerUtils/profiler.h"
#include "OpenViewerUtils/hash.h"

int main(int argc, char** argv)
{
    SET_SPDLOG_FMT;
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Starting Media Test");

    try
    {
        lov::MediaPool media_pool;

        media_pool.add_media(fmt::format("{}/exr/sequence/compo_0500.0100.exr", TEST_DATA_DIR));
        media_pool.add_media(fmt::format("{}/exr/exr_multilayers.exr", TEST_DATA_DIR));
        media_pool.add_media(fmt::format("{}/png/png_4K.png", TEST_DATA_DIR));
        media_pool.add_media(fmt::format("{}/cc_skull_test_image/CCSkull_Linear709.exr", TEST_DATA_DIR));

        media_pool.debug_media();

        media_pool.remove_media(fmt::format("{}/cc_skull_test_image/CCSkull_Linear709.exr", TEST_DATA_DIR));
        
        media_pool.debug_media();
    }
    catch(const std::exception& err)
    {
        spdlog::error("Test Failed, caught exception : \n  {}", err.what());
        
        std::exit(EXIT_FAILURE);
    }

    spdlog::info("Test Passed");

    return EXIT_SUCCESS;
}