// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/openviewer.h"
#include "OpenViewer/settings.h"
#include "OpenViewer/cache.h"
#include "OpenViewer/media_pool.h"

int main(int argc, char** argv)
{
    SET_SPDLOG_FMT;
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Starting Cache Test");

    try
    {
        lov::Cache cache;

        lov::MediaPool media_pool;

        const std::string path = fmt::format("{}/exr/sequence/compo_0500.0100.exr", TEST_DATA_DIR);
        media_pool.add_media(path);

        lov::Media* img_seq = media_pool.get_media(0);

        img_seq->debug();

        const uint64_t seq_byte_size = img_seq->get_media_byte_size();

        cache.resize(seq_byte_size);

        for(uint8_t i = 100; i < 220; i++)
        {
            cache.add(img_seq, i);
        }

        cache.debug();

        img_seq->debug();
        
        for(uint8_t i = 100; i < 244; i++)
        {
            if(img_seq->is_cached_at_frame(i)) continue;
            
            cache.add(img_seq, i);
        }

        cache.debug();

        img_seq->debug();
    }
    catch(const std::exception& err)
    {
        spdlog::error("Test Failed, catched exception : \n  {}", err.what());
        
        std::exit(EXIT_FAILURE);
    }

    spdlog::info("Test Passed");

    return EXIT_SUCCESS;
}
