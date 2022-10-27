// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/openviewer.h"
#include "OpenViewer/media.h"
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
        const std::string file_seq = lovu::func_timer(lovu::fs::get_file_sequence_from_file, 
                                                      fmt::format("{}/exr/sequence/compo_0500.0100.exr", 
                                                      TEST_DATA_DIR));

        lov::ImageSequence image_seq(file_seq);

        {
            SCOPED_TIMER("Hash");

            const uint32_t hash120 = image_seq.get_hash_at_frame(120);
            const uint32_t hash120_2 = image_seq.get_hash_at_frame(120);

            spdlog::debug("{}", hash120 == hash120_2);

            spdlog::debug("Hash 120 : {}", hash120);
            spdlog::debug("Hash2 120 : {}", hash120_2);

            const uint32_t hash121 = image_seq.get_hash_at_frame(121);
            
            spdlog::debug("{}", hash120 != hash121);

            spdlog::debug("Hash 121 : {}", hash121);
        }
    }
    catch(const std::exception& err)
    {
        spdlog::error("Test Failed, catched exception : \n  {}", err.what());
        
        std::exit(EXIT_FAILURE);
    }

    spdlog::info("Test Passed");

    return EXIT_SUCCESS;
}