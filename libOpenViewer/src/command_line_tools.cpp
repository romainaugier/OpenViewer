// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/command_line_tools.h"
#include "OpenViewer/ocio.h"
#include "OpenViewer/media_pool.h"
#include "OpenViewer/cache.h"
#include "OpenViewerUtils/filesystem.h"

#include "OpenImageIO/argparse.h"
#include "OpenImageIO/imagebuf.h"
#include "OpenImageIO/imagebufalgo.h"
#include "tbb/parallel_for.h"

LOV_NAMESPACE_BEGIN

void LOV_API process_command_line_args(int argc, char** argv) noexcept
{
    spdlog::debug("Processing command line arguments");
    
    // Options that can be passed as command line args
    static std::string command = "";
    static std::string file = "";
    static std::string ocio_role = "";
    static std::string ocio_display = "";
    static std::string ocio_view = "";
    static int thumbnail_size = 128;

    static bool help = false;

    static OIIO::ArgParse parser;
    
    parser.options("OpenViewer command line tools -- \n"
                   "  Usage : openviewer -command <command> [options]\n"
                   "\n"
                   "  Available commands : \n"
                   "    - ocio_convert : copy images with colorimetric space conversion using OpenColorIO\n"
                   "      > usage : openviewer ocio_convert --file C:/path/to/image_0100_001.exr\n"
                   "    - make_thumbnail : creates a thumbnail of the given image and size\n"
                   "      > usage : openviewer make_thumbnail --file C:/path/to/image_00.exr --size 256\n"
                   "\n",
                   "-command %s", &command, "Command to execute",
                   "--help", &help, "Print help message",
                   "--file %s", &file, "File/File sequence to convert",
                   "--ocio_role %s", &ocio_role, "Ocio role",
                   "--ocio_display %s", &ocio_display, "Ocio display",
                   "--ocio_view %s", &ocio_view, "Ocio view",
                   "--size %d", &thumbnail_size, "Thumbnail Size",
                   nullptr);

    if(parser.parse_args(argc, (const char**)argv) < 0)
    {
        spdlog::error("Error during command line arguments parsing : {}", parser.geterror());
        std::exit(EXIT_FAILURE);
    }

    if(help)
    {
        spdlog::info("Help");
        std::exit(EXIT_SUCCESS);
    }

    if(command == "ocio_convert")
    {
        if(file == "")
        {
            spdlog::error("Error during command line arguments parsing. Specified command <ocio_convert>"
                          "but an argument \"file\" is missing.");
            std::exit(EXIT_FAILURE);
        }
        if(ocio_role == "")
        {
            spdlog::error("Error during command line arguments parsing. Specified command <ocio_convert>"
                          "but an argument \"ocio_role\" is missing.");
            std::exit(EXIT_FAILURE);
        }
        if(ocio_display == "")
        {
            spdlog::error("Error during command line arguments parsing. Specified command <ocio_convert>"
                          "but an argument \"ocio_display\" is missing.");
            std::exit(EXIT_FAILURE);
        }
        if(ocio_view == "")
        {
            spdlog::error("Error during command line arguments parsing. Specified command <ocio_convert>"
                          "but an argument \"ocio_view\" is missing.");
            std::exit(EXIT_FAILURE);
        }

        ocio_convert(file, ocio_role, ocio_display, ocio_view, true);
    }
    else if(command == "make_thumbnail")
    {
        if(file == "")
        {
            spdlog::error("Error during command line parsing. Specified command <make_thumbnail>"
                          "but an argument \"file\" is missing.");
            std::exit(EXIT_FAILURE);
        }

        make_thumbnail(file, thumbnail_size);
    }
}

void LOV_API ocio_convert(std::string& filename, 
                          const std::string& ocio_role,
                          const std::string& ocio_display,
                          const std::string& ocio_view,
                          const bool autodetect_sequence) noexcept
{
//     spdlog::debug("Starting OCIO Convert command");
    
//     Ocio ocio;
    
//     ocio.set_role_from_str(ocio_role.c_str());
//     ocio.set_display_from_str(ocio_display.c_str());
//     ocio.set_view_from_str(ocio_view.c_str());

//     ocio.update_processor();

//     MediaPool media_pool;

//     media_pool.add_media(filename);

//     Media* media = media_pool.get_media(0);
//     media->debug();
    
//     ocio.update_cpu_processor(media->get_ocio_bitdepth());

//     Cache cache;
//     cache.resize(media->get_media_byte_size());

//     const bool multithread = true;

//     if(multithread)
//     {
//         static tbb::affinity_partitioner partitioner;

//         tbb::parallel_for(tbb::blocked_range<size_t>(media->get_start_frame(), (media->get_end_frame() + 1)), [&](const tbb::blocked_range<size_t>& r)
//         {
//             for(size_t i = r.begin(), i_end = r.end(); i < i_end; i++)
//             {
//                 try
//                 {
//                     const std::string origin_path = media->make_path_at_frame(i);

//                     if(!lovu::fs::exists(origin_path))
//                     {
//                         continue;
//                     }

//                     if(!lovu::str::ends_with(origin_path, ".exr"))
//                     {
//                         spdlog::warn("File \"{}\" is not an OpenEXR file, discarding it", origin_path);
//                         continue;
//                     }

//                     void* cache_address = cache.add(media, i);

//                     media->load_frame_to_memory(cache_address, i);

//                     ocio.process_cpu(cache_address, 
//                                     media->get_width(), 
//                                     media->get_height(), 
//                                     media->get_nchannels(), 
//                                     media->get_type());

//                     OIIO::ImageSpec specs(media->get_width(), 
//                                         media->get_height(), 
//                                         media->get_nchannels(),
//                                         media->get_oiio_typedesc());

//                     OIIO::ImageBuf buffer(specs, cache_address);

//                     std::string file_name = std::filesystem::path(origin_path).filename().string();

//                     lovu::str::re_replace(file_name, std::regex("\\.exr"), ".png");

//                     const std::string output_path = fmt::format("{}/aces_convert/{}", lovu::fs::get_parent_dir(origin_path), file_name);

//                     if(!lovu::fs::exists(output_path))
//                     {
//                         lovu::fs::makedirs(output_path);
//                     }

//                     if(!buffer.write(output_path))
//                     {
//                         spdlog::error("OIIO Error during file write : {}", buffer.geterror());
//                     }
//                 }
//                 catch(const std::exception& e)
//                 {
//                     spdlog::error("Exception catched : {}", e.what());
//                 }
//             }
//         }, partitioner);
//     }
//     else
//     {
//         for(size_t i = media->get_start_frame(); i < (media->get_end_frame() + 1); i++)
//         {
//             try
//             {
//                 const std::string origin_path = media->make_path_at_frame(i);

//                 if(!lovu::fs::exists(origin_path))
//                 {
//                     continue;
//                 }

//                 if(!lovu::str::ends_with(origin_path, ".exr"))
//                 {
//                     spdlog::warn("File \"{}\" is not an OpenEXR file, discarding it", origin_path);
//                     continue;
//                 }

//                 void* cache_address = cache.add(media, i);
//                 media->load_frame_to_memory(cache_address, i);

//                 ocio.process_cpu(cache_address, 
//                                  media->get_width(), 
//                                  media->get_height(), 
//                                  media->get_nchannels(), 
//                                  media->get_type());

//                 OIIO::ImageSpec specs(media->get_width(), 
//                                       media->get_height(), 
//                                       media->get_nchannels(),
//                                       media->get_oiio_typedesc());

//                 OIIO::ImageBuf buffer(specs, cache_address);

//                 std::string file_name = std::filesystem::path(origin_path).filename().string();

//                 lovu::str::re_replace(file_name, std::regex("\\.exr"), ".png");

//                 const std::string output_path = fmt::format("{}/aces_convert/{}", lovu::fs::get_parent_dir(origin_path), file_name);

//                 if(!lovu::fs::exists(output_path))
//                 {
//                     lovu::fs::makedirs(output_path);
//                 }

//                 if(!buffer.write(output_path))
//                 {
//                     spdlog::error("OIIO Error during file write : {}", buffer.geterror());
//                 }
//             }
//             catch(const std::exception& e)
//             {
//                 spdlog::error("Exception catched : {}", e.what());
//             }
//         }
//     }

//     spdlog::debug("Finished OCIO Convert command");
}

void LOV_API make_thumbnail(std::string& filename,
                            const uint16_t thumbnail_size) noexcept
{
    spdlog::debug("Starting Make Thumbnail command");

    try
    {
        OIIO::ImageBuf source(filename);


        OIIO::ROI roi(0, thumbnail_size, 0, thumbnail_size);

        OIIO::ImageBuf dest = OIIO::ImageBufAlgo::fit(source, "", 0, true, roi);

        const std::string out_file_path = fmt::format("{}/{}_thumbnail.png",
                                                      lovu::fs::get_parent_dir(filename),
                                                      lovu::fs::get_filename_no_ext(filename));

        if(!dest.write(out_file_path))
        {
            spdlog::error("Error during thumbnail write : {}", dest.geterror());
        }
        else
        {
            spdlog::info("Wrote thumbnail file to : \"{}\"", out_file_path);
        }
    }
    catch(const std::exception& e)
    {
        spdlog::error("Exception catched : {}", e.what());
    }

    spdlog::debug("Finished Make Thumbnail command");
}

LOV_NAMESPACE_END