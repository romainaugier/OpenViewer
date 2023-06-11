// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/media.h"
#include "OpenViewer/cache.h"
#include "OpenViewer/media_pool.h"
#include "OpenViewer/display.h"

#include "GL/glew.h"

#include "GLFW/glfw3.h"

using namespace lov;

void glfw_error_callback(int code, const char* desc)
{
    spdlog::error("Glfw error : {} ({})", desc, code);
}

int main(int argc, char** argv)
{
    SET_SPDLOG_FMT;
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Starting Display Test");

    glfwSetErrorCallback(glfw_error_callback);

    if(!glfwInit())
    {
        spdlog::error("Failed to initialize glfw");
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
 
    GLFWwindow* offscreen_context = glfwCreateWindow(640, 480, "", NULL, NULL);

    if(!offscreen_context)
    {
        spdlog::error("Failed to create glfw window");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(offscreen_context);

    GLenum err = glewInit();

    if(err != GLEW_OK)
    {
        spdlog::error("Failed to initialize glew : {}", glewGetErrorString(err));
        glfwTerminate();
        return EXIT_FAILURE;
    }

    try
    {
        lov::Display display;

        lov::Cache cache;

        lov::MediaPool media_pool;

        const std::string path = fmt::format("{}/exr/sequence/compo_0500.0100.exr", TEST_DATA_DIR);
        media_pool.add_media(path);

        lov::Media* img_seq = media_pool.get_media(0);

        const uint64_t seq_byte_size = img_seq->get_media_byte_size();

        cache.resize(seq_byte_size);

        for(uint32_t i = img_seq->get_start_frame(); i < img_seq->get_end_frame(); i++)
        {
            void* cache_address = cache.add(img_seq, i);
            img_seq->load_frame_to_cache(cache_address, i);
        }
        
        for(uint32_t i = img_seq->get_start_frame(); i < img_seq->get_end_frame(); i++)
        {
            if(!img_seq->is_cached_at_frame(i))
            {
                void* cache_address = cache.add(img_seq, i);
                img_seq->load_frame_to_cache(cache_address, i);
            }

            const uint8_t img_fmt = FORMAT_FROM_NCHANNELS(img_seq->get_nchannels());

            lov::cache_item* cache_item = cache.get_cache_item(img_seq->get_hash_at_frame(i));

            if(cache_item == nullptr)
            {
                spdlog::debug("Image not in cache at frame {}", i);
                continue;
            }

            display.set_data(cache_item->m_data_ptr,
                             img_seq->get_width(),
                             img_seq->get_height(),
                             (uint8_t)TYPE_AND_FMT_TO_DISPLAY_DATA_TYPE(img_seq->get_type(),
                                                                        img_fmt));
        }
    }
    catch(const std::exception& err)
    {
        spdlog::error("Test Failed, caught exception : \n {}", err.what());

        return EXIT_FAILURE;
    }

    glfwDestroyWindow(offscreen_context);
    glfwTerminate();

    spdlog::info("Test Passed");

    return EXIT_SUCCESS;
}