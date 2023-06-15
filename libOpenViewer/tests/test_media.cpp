// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/openviewer.h"
#include "OpenViewer/media.h"
#include "OpenViewerUtils/filesystem.h"
#include "OpenViewerUtils/profiler.h"
#include "OpenViewerUtils/hash.h"
#include "spdlog/spdlog.h"

int main(int argc, char** argv)
{
    SET_SPDLOG_FMT;
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Starting Media Test");

    try
    {
        const std::string file_seq = lovu::func_timer(lovu::fs::get_file_sequence_from_file, 
                                                      fmt::format("{}/exr/sequence/compo_0500.0100.exr", 
                                                      TEST_DATA_DIR));

        lov::ImageSequence image_seq(file_seq);
        image_seq.debug();

        lov::Image jpg_image(fmt::format("{}/jpg/jpg_20K.jpg", TEST_DATA_DIR));
        jpg_image.debug();

        lov::Image png_image(fmt::format("{}/png/png_4K.png", TEST_DATA_DIR));
        png_image.debug();

        lov::Image png_image_rgb_8(fmt::format("{}/png/png_rgb8bits.png", TEST_DATA_DIR));
        png_image_rgb_8.debug();

        lov::Image png_image_rgb_16(fmt::format("{}/png/png_rgb16bits.png", TEST_DATA_DIR));
        png_image_rgb_16.debug();

        lov::Image png_image_rgba_8(fmt::format("{}/png/png_rgba8bits.png", TEST_DATA_DIR));
        png_image_rgba_8.debug();

        lov::Image png_image_rgba_16(fmt::format("{}/png/png_rgba16bits.png", TEST_DATA_DIR));
        png_image_rgba_16.debug();
    }
    catch(const std::exception& err)
    {
        spdlog::error("Test Failed, catched exception : \n  {}", err.what());
        
        return EXIT_FAILURE;
    }

    spdlog::info("Test Passed");

    return EXIT_SUCCESS;
}
