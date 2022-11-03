// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/openviewer.h"
#include "OpenViewer/input.h"
#include "OpenViewer/cache.h"
#include "OpenViewer/media_pool.h"
#include "OpenViewerUtils/profiler.h"

int main(int argc, char** argv)
{
    SET_SPDLOG_FMT;
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Starting Input Test");

    try
    {
        lov::Cache cache;

        lov::MediaPool media_pool;

        media_pool.add_media(fmt::format("{}/exr/sequence/compo_0500.0100.exr", TEST_DATA_DIR));

        lov::Media* img_seq = media_pool.get_media(0);

        img_seq->debug();

        const uint64_t seq_byte_size = img_seq->get_media_byte_size();

        spdlog::debug(seq_byte_size);

        cache.resize(seq_byte_size);

        {
            lovu::ScopedTimer("EXR Cache load");

            for(uint8_t i = 100; i < 220; i++)
            {
                void* cache_address = cache.add(img_seq, i);
                img_seq->load_frame_to_cache(cache_address, i);

                const uint32_t hash = img_seq->get_hash_at_frame(i);
                const lov::cache_item* cache_item = cache.get_cache_item(hash);
                
                if(cache_item == nullptr)
                {
                    spdlog::warn("Found nullptr");
                }
            }
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