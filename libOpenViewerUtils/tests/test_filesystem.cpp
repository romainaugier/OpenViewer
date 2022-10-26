// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewerUtils/filesystem.h"
#include "OpenViewerUtils/profiler.h"

int main(int argc, char** argv)
{
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Starting FileSystem Test");

    try
    {
        std::vector<std::string> file_names;

        lovu::func_timer(lovu::fs::get_filenames_from_dir, 
                         file_names, 
                         fmt::format("{}/exr/sequence", TEST_DATA_DIR));

        for(const auto& file_name : file_names)
        {
            spdlog::info("Found filename : {}", file_name);
        }

        const std::string file_seq = lovu::func_timer(lovu::fs::get_file_sequence_from_file, 
                                                      fmt::format("{}/exr/sequence/compo_0500.0100.exr", 
                                                      TEST_DATA_DIR));

        if(file_seq != "")
        {
            spdlog::info("Found file sequence : {}", file_seq);
        }
    }
    catch(const std::exception& err)
    {
        spdlog::error("Test Failed, catched exception : \n  {}", err.what());
        
        std::exit(EXIT_FAILURE);
    }

    spdlog::info("Test passed");
    return EXIT_SUCCESS;
}