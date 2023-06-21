// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/openviewer.h"
#include "OpenViewer/input.h"
#include "OpenViewer/cache.h"
#include "OpenViewer/media_pool.h"
#include "OpenViewer/ocio.h"
#include "OpenViewerUtils/profiler.h"
#include "OpenViewerUtils/filesystem.h"

#include "OpenImageIO/imagebuf.h"
#include "tbb/parallel_for.h"

int main(int argc, char** argv)
{
    SET_SPDLOG_FMT;
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Starting OCIO Test");

    try
    {
        lov::Ocio ocio;

        ocio.set_current_role(8);
        ocio.set_current_display(0);
        ocio.set_current_view(3);

        ocio.update_processor();

        ocio.debug();

        lov::Cache cache;

        lov::MediaPool media_pool;

        media_pool.add_media(fmt::format("{}/exr/sequence/compo_0500.0100.exr", TEST_DATA_DIR));

        lov::ImageSequence* img_seq = dynamic_cast<lov::ImageSequence*>(media_pool.get_media(0));
        
        ocio.update_cpu_processor(img_seq->get_ocio_bitdepth());

        img_seq->debug();

        const uint64_t seq_byte_size = img_seq->get_media_byte_size();

        cache.resize(seq_byte_size);

        {
            lovu::ScopedTimer("EXR Cache load");

            static tbb::affinity_partitioner partitioner;

            tbb::parallel_for(tbb::blocked_range<size_t>(100, 220), [&](const tbb::blocked_range<size_t>& r)
            {
                for(size_t i = r.begin(), i_end = r.end(); i < i_end; i++)
                {
                    void* cache_address = cache.add(img_seq, i);

                    ocio.process_cpu(cache_address, 
                                    img_seq->get_width(), 
                                    img_seq->get_height(), 
                                    img_seq->get_nchannels(), 
                                    img_seq->get_type());


                    OIIO::ImageSpec specs(img_seq->get_width(), 
                                          img_seq->get_height(), 
                                          img_seq->get_nchannels(),
                                          nullptr);

                    OIIO::ImageBuf buffer(specs, cache_address);

                    const std::string origin_path = img_seq->make_path_at_frame(i);

                    std::string file_name = std::filesystem::path(origin_path).filename().string();

                    lovu::str::re_replace(file_name, std::regex("\\.exr"), ".png");

                    const std::string output_path = fmt::format("{}/output/{}", lovu::fs::get_parent_dir(origin_path), file_name);

                    if(!lovu::fs::exists(output_path))
                    {
                        lovu::fs::makedirs(output_path);
                    }

                    buffer.write(output_path);
                }
            }, partitioner);
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