// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/image.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MALLOC stdromano::mem_alloc
#define STBI_REALLOC stdromano::mem_realloc
#define STBI_FREE stdromano::mem_free
#include "stb_image_write.hpp"

#include "stdromano/logger.hpp"

#include "OpenEXR/ImfHeader.h"
#include "OpenEXR/ImfFrameBuffer.h"
#include "OpenEXR/ImfChannelList.h"
#include "OpenEXR/ImfOutputFile.h"

LOV_NAMESPACE_BEGIN

/* Jpeg */

bool image_write_jpg_from_rgba(const stdromano::StringD& path, Image& img) noexcept
{
    Layer* rgba_layer = img.main();

    if(rgba_layer == nullptr)
    {
        stdromano::log_error("Error during write of image {}, no main layer has been found");
        return false;
    }

    Layer rgb_layer = *rgba_layer;

    rgb_layer.shuffle("RGB");

    if(rgb_layer.depth() != LayerDepth_U8)
    {
        rgb_layer.convert(LayerDepth_U8);
    }

    if(stbi_write_jpg(path.c_str(),
                      img.get_display_width(),
                      img.get_display_height(),
                      rgb_layer.nchannels(),
                      rgb_layer.data<void>(),
                      100) == 0)
    {
        stdromano::log_error("Error during write of image {}", path);
        return false;
    }

    return true;
}

bool image_write_jpg(const stdromano::StringD& path, Image& img) noexcept
{
    Layer* layer = img.main();

    if(layer->nchannels() == 4)
    {
        return image_write_jpg_from_rgba(path, img);
    }
    else if(layer->nchannels() < 3)
    {
        stdromano::log_error("Cannot write a jpeg \"{}\" image with less than 3 channels", path);
        return false;
    }

    if(layer->depth() != LayerDepth_U8)
    {
        stdromano::log_debug("Converting image {} to rgb u8 before writing", path);

        Layer new_layer = *layer;
        new_layer.convert(LayerDepth_U8);

        if(stbi_write_jpg(path.c_str(),
                          img.get_display_width(),
                          img.get_display_height(),
                          new_layer.nchannels(),
                          new_layer.data<void>(),
                          100) == 0)
        {
            stdromano::log_error("Error during write of image {}", path);
            return false;
        }
    }
    else
    {
        if(stbi_write_jpg(path.c_str(),
                          img.get_display_width(),
                          img.get_display_height(),
                          layer->nchannels(),
                          layer->data<void>(),
                          100) == 0)
        {
            stdromano::log_error("Error during write of image {}", path);
            return false;
        }
    }

    stdromano::log_debug("Wrote image {}", path);

    return true;
}

/* PNG */

bool image_write_png(const stdromano::StringD& path, Image& img) noexcept
{
    const Layer* layer = img.main();

    if(layer->nchannels() < 3)
    {
        stdromano::log_error("Cannot write a png image \"{}\" with less than 3 channels", path);
        return false;
    }

    if(layer->depth() != LayerDepth_U8)
    {
        stdromano::log_debug("Converting image {} to rgb u8 before writing", path);

        Layer new_layer = *layer;
        new_layer.convert(LayerDepth_U8);

        if(stbi_write_png(path.c_str(),
                          img.get_display_width(),
                          img.get_display_height(),
                          new_layer.nchannels(),
                          new_layer.data<void>(),
                          img.get_display_width() * new_layer.channel_stride()) == 0)
        {
            stdromano::log_error("Error during write of image {}", path);
            return false;
        }
    }
    else
    {
        if(stbi_write_png(path.c_str(),
                          img.get_display_width(),
                          img.get_display_height(),
                          layer->nchannels(),
                          layer->data<void>(),
                          img.get_display_width() * layer->channel_stride()) == 0)
        {
            stdromano::log_error("Error during write of image {}", path);
            return false;
        }
    }

    return true;
}

/* HDR */

bool image_write_hdr(const stdromano::StringD& path, Image& img) noexcept
{
    const Layer* layer = img.main();

    if(layer->nchannels() < 3)
    {
        stdromano::log_error("Cannot write an hdr image \"{}\" with less than 3 channels",
                             path);
        return false;
    }

    if(layer->depth() != LayerDepth_F32)
    {
        Layer new_layer = *layer;
        new_layer.convert(LayerDepth_F32);

        if(stbi_write_hdr(path.c_str(),
                          img.get_display_width(),
                          img.get_display_height(),
                          new_layer.nchannels(),
                          new_layer.data<float>()) == 0)
        {
            stdromano::log_error("Error during write of image {}", path);
            return false;
        }
    }
    else
    {
        if(stbi_write_hdr(path.c_str(),
                          img.get_display_width(),
                          img.get_display_height(),
                          layer->nchannels(),
                          layer->data<float>()) == 0)
        {
            stdromano::log_error("Error during write of image {}", path);
            return false;
        }
    }

    return true;
}

/* EXR */

static const char* const exr_channels = "RGBA";

bool image_write_exr(const stdromano::StringD& path, Image& img) noexcept
{
    Imf::Header header(img.display_window(),
                       img.data_window(),
                       img.aspect_ratio(),
                       Imath::V2f(0.0f, 0.0f),
                       1.0f,
                       Imf::LineOrder::INCREASING_Y,
                       Imf::Compression::PIZ_COMPRESSION);

    for(const auto& [layer_name, layer] : img.get_layers())
    {
        if(layer.depth() < LayerDepth_F16)
        {
            stdromano::log_error("EXR does not support layers with depth other than F16 or F32");
            return false;
        }

        const Imf::PixelType pixel_type = layer.depth() == LayerDepth_F32 ? Imf::FLOAT :
                                                                            Imf::HALF;

        if(layer_name == Image::MAIN_LAYER_NAME)
        {
            for(std::size_t i = 0; i < layer.nchannels(); i++)
            {
                header.channels().insert(std::string(1, exr_channels[i]), Imf::Channel(pixel_type));
            }
        }
        else
        {
            for(std::size_t i = 0; i < layer.nchannels(); i++)
            {
                const stdromano::String260 channel_name("{}.{}", layer_name, exr_channels[i]);
                header.channels().insert(channel_name.c_str(), Imf::Channel(pixel_type));
            }
        }
    }

    Imf::OutputFile file(path.c_str(), header);

    for(const auto& [layer_name, layer] : img.get_layers())
    {
        Imf::FrameBuffer frame_buffer;

        if(layer.depth() < LayerDepth_F16)
        {
            stdromano::log_error("EXR does not support layers with depth other than F16 or F32");
            return false;
        }

        const Imf::PixelType pixel_type = layer.depth() == LayerDepth_F32 ? Imf::FLOAT :
                                                                            Imf::HALF;

        const std::size_t pixel_size = layer_depth_as_byte_size(layer.depth());
        std::size_t offset = 0;

        if(layer_name == Image::MAIN_LAYER_NAME)
        {
            for(std::size_t i = 0; i < layer.nchannels(); i++)
            {
                frame_buffer.insert(std::string(1, exr_channels[i]),
                                    Imf::Slice(pixel_type,
                                               const_cast<char*>(layer.data<char>()) + offset,
                                               pixel_size * layer.nchannels(),
                                               pixel_size * layer.nchannels() * img.get_data_width()));

                offset += pixel_size;
            }
        }
        else
        {
            for(std::size_t i = 0; i < layer.nchannels(); i++)
            {
                const stdromano::String260 channel_name("{}.{}", layer_name, exr_channels[i]);

                frame_buffer.insert(channel_name.c_str(),
                                    Imf::Slice(pixel_type,
                                               const_cast<char*>(layer.data<char>()) + offset,
                                               pixel_size * layer.nchannels(),
                                               pixel_size * layer.nchannels() * img.get_data_width()));

                offset += pixel_size;
            }
        }

        file.setFrameBuffer(frame_buffer);
        file.writePixels(img.get_data_height());
    }

    return true;
}

/* Dispatcher */

using ImageWriteFunc = bool(*)(const stdromano::StringD&, Image&) noexcept;

stdromano::HashMap<stdromano::StringD, ImageWriteFunc> g_write_funcs_table = {
    { "jpg", image_write_jpg },
    { "jpeg", image_write_jpg },
    { "png", image_write_png },
    { "hdr", image_write_hdr },
    { "exr", image_write_exr },
};

bool Image::write(const stdromano::StringD& path) noexcept
{
    const stdromano::StringD ext = path.rsplit(".");

    auto it = g_write_funcs_table.find(ext);

    if(it == g_write_funcs_table.end())
    {
        stdromano::log_error("No function available to write file: {} (type is: {})", path, ext);
        return false;
    }

    return it->second(path, *this);
}

LOV_NAMESPACE_END
