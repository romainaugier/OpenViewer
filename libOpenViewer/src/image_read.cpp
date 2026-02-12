// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/media.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_MALLOC stdromano::mem_alloc
#define STBI_REALLOC stdromano::mem_realloc
#define STBI_FREE stdromano::mem_free
#include "stb_image_read.hpp"

#include "stdromano/logger.hpp"
#include "stdromano/vector.hpp"
#include "stdromano/filesystem.hpp"

#include "OpenEXR/ImfInputFile.h"
#include "OpenEXR/ImfChannelList.h"
#include "OpenEXR/ImfFrameBuffer.h"
#include "Imath/half.h"
#include "Imath/ImathBox.h"

#include "tiffio.h"

#include <cstdio>

LOV_NAMESPACE_BEGIN

/* JPEG */

bool image_read_info_jpeg(const stdromano::StringD& path,
                          MediaInfo& info) noexcept
{
    int x, y, n;

    if(stbi_info(path.c_str(), &x, &y, &n) == 0)
    {
        stdromano::log_error("Error while loading image: \"{}\"", path);
        return false;
    }

    info = MediaInfo(Imath::Box2i(Imath::V2i(0, 0), Imath::V2i(x - 1, y - 1)),
                     Imath::Box2i(Imath::V2i(0, 0), Imath::V2i(x - 1, y - 1)),
                     static_cast<float>(x) / static_cast<float>(y));

    info.create_layer(MediaInfo::MAIN_LAYER_NAME,
                      static_cast<MediaFormat>(n - 1),
                      MediaDepth_U8);

    return true;
}

bool pixels_read_jpeg(const stdromano::StringD& path,
                      const stdromano::StringD& layer_name,
                      const MediaLayer& layer,
                      void* data) noexcept
{
    // TODO

    int x, y, n;

    // void* data = static_cast<void*>(stbi_load(path.c_str(), &x, &y, &n, 0));

    // if(data == nullptr)
    // {
    //     stdromano::log_error("Error while loading layer \"{}\" from image: \"{}\"",
    //                          layer_name,
    //                          path);

    //     return false;
    // }

    return true;
}

/* PNG */

bool image_read_info_png(const stdromano::StringD& path,
                         MediaInfo& info) noexcept
{
    int x, y, n;

    if(stbi_info(path.c_str(), &x, &y, &n) == 0)
    {
        stdromano::log_error("Error while loading image: \"{}\"", path);
        return false;
    }

    info = MediaInfo(Imath::Box2i(Imath::V2i(0, 0), Imath::V2i(x - 1, y - 1)),
                     Imath::Box2i(Imath::V2i(0, 0), Imath::V2i(x - 1, y - 1)),
                     static_cast<float>(x) / static_cast<float>(y));

    info.create_layer(MediaInfo::MAIN_LAYER_NAME,
                        static_cast<MediaFormat>(n - 1),
                        MediaDepth_U8);

    return true;
}

bool pixels_read_png(const stdromano::StringD& path,
                     const stdromano::StringD& layer_name,
                     const MediaLayer& layer,
                     void* data) noexcept
{
    int x, y, n;

    // void* data = stbi_load(path.c_str(), &x, &y, &n, 0);

    // if(data == nullptr)
    // {
    //     stdromano::log_error("Error while loading layer \"{}\" from image: \"{}\"",
    //                          layer_name,
    //                          path);

    //     return false;
    // }

    return true;
}

/* HDR */

bool image_read_info_hdr(const stdromano::StringD& path,
                         MediaInfo& info) noexcept
{
    int x, y, n;

    if(stbi_info(path.c_str(), &x, &y, &n) == 0)
    {
        stdromano::log_error("Error while loading image: \"{}\"", path);
        return false;
    }

    info = MediaInfo(Imath::Box2i(Imath::V2i(0, 0), Imath::V2i(x - 1, y - 1)),
                     Imath::Box2i(Imath::V2i(0, 0), Imath::V2i(x - 1, y - 1)),
                     static_cast<float>(x) / static_cast<float>(y));

    info.create_layer(MediaInfo::MAIN_LAYER_NAME,
                      static_cast<MediaFormat>(n - 1),
                      MediaDepth_F32);

    return true;
}

bool pixels_read_hdr(const stdromano::StringD& path,
                     const stdromano::StringD& layer_name,
                     const MediaLayer& layer,
                     void* data) noexcept
{
    // TODO

    int x, y, n;

    float* img_data = stbi_loadf(path.c_str(), &x, &y, &n, 0);

    if(img_data == nullptr)
    {
        stdromano::log_error("Error while loading layer \"{}\" from image: \"{}\"",
                             layer_name,
                             path);
        return false;
    }

    return true;
}

/* Tiff */

#define TIFF_HANDLER_BUF_SIZE 1024

void tiff_error_handler(const char* module, const char* fmt, va_list ap)
{
    LOV_UNUSED(module);

    char buf[TIFF_HANDLER_BUF_SIZE];
    std::memset(buf, 0, sizeof(buf));

    vsnprintf(buf, TIFF_HANDLER_BUF_SIZE, fmt, ap);

    stdromano::log_error("{}", buf);
}

void tiff_warning_handler(const char* module, const char* fmt, va_list ap)
{
    LOV_UNUSED(module);

    char buf[TIFF_HANDLER_BUF_SIZE];
    std::memset(buf, 0, sizeof(buf));

    vsnprintf(buf, TIFF_HANDLER_BUF_SIZE, fmt, ap);

    stdromano::log_warn("{}", buf);
}

bool image_read_info_tiff(const stdromano::StringD& path,
                          MediaInfo& info) noexcept
{
    TIFFSetErrorHandler(tiff_error_handler);
    TIFFSetWarningHandler(tiff_warning_handler);

    TIFF* tif = TIFFOpen(path.c_str(), "r");

    if(tif == nullptr)
    {
        stdromano::log_error("Error while loading image: \"{}\"", path);
        return false;
    }

    std::uint32_t width, height;

    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

    std::uint16_t n_channels = 1, bits_per_sample = 1, sample_format = SAMPLEFORMAT_UINT;
    TIFFGetFieldDefaulted(tif, TIFFTAG_SAMPLESPERPIXEL, &n_channels);
    TIFFGetFieldDefaulted(tif, TIFFTAG_BITSPERSAMPLE, &bits_per_sample);
    TIFFGetFieldDefaulted(tif, TIFFTAG_SAMPLEFORMAT, &sample_format);

    MediaDepth depth;

    switch(sample_format)
    {
        case SAMPLEFORMAT_UINT:
            switch(bits_per_sample)
            {
                case 8:
                    depth = MediaDepth_U8;
                    break;
                case 16:
                    depth = MediaDepth_U16;
                    break;
                case 32:
                    depth = MediaDepth_U32;
                    break;
                default:
                    stdromano::log_error("Unknown uint bit depth: {}", bits_per_sample);
                    return false;
            }

            break;

        case SAMPLEFORMAT_IEEEFP:
            switch(bits_per_sample)
            {
                case 16:
                    depth = MediaDepth_F16;
                    break;
                case 32:
                    depth = MediaDepth_F32;
                    break;
                default:
                    stdromano::log_error("Unknown float bit depth: {}", bits_per_sample);
                    return false;
            }

            break;

        default:
            stdromano::log_error("Unsupported sample format: {}", sample_format);
            return false;
    }

    info = MediaInfo(Imath::Box2i(Imath::V2i(0, 0), Imath::V2i(width - 1, height - 1)),
                     Imath::Box2i(Imath::V2i(0, 0), Imath::V2i(width - 1, height - 1)),
                     static_cast<float>(width) / static_cast<float>(height));

    info.create_layer(MediaInfo::MAIN_LAYER_NAME,
                      static_cast<MediaFormat>(n_channels - 1),
                      depth);

    TIFFClose(tif);

    return true;
}

bool pixels_read_tiff(const stdromano::StringD& path,
                      const stdromano::StringD& layer_name,
                      const MediaLayer& layer,
                      void* data) noexcept
{
    LOV_UNUSED(layer);

    TIFF* tif = TIFFOpen(path.c_str(), "r");

    std::uint32_t width, height;

    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

    if(!TIFFReadRGBAImage(tif,
                          width,
                          height,
                          static_cast<std::uint32_t*>(data),
                          0))
    {
        TIFFClose(tif);

        stdromano::log_error("Error while loading layer \"{}\" from image: \"{}\"",
                             layer_name,
                             path);

        return false;
    }

    TIFFClose(tif);

    return true;
}

/* EXR */

using EXRLayerNames = stdromano::HashMap<stdromano::StringD, stdromano::Vector<stdromano::StringD>>;

EXRLayerNames image_get_layer_names_exr(const Imf::ChannelList& channels) noexcept
{
    EXRLayerNames layer_names;

    for(Imf::ChannelList::ConstIterator it = channels.begin(); it != channels.end(); ++it)
    {
        stdromano::StringD name;
        stdromano::StringD channel_name = stdromano::StringD::make_ref(it.name()).rsplit(".",
                                                                                         &name);

        if(name.size() == 0)
        {
            layer_names["default"].push_back(channel_name);
        }
        else
        {
            layer_names[name].push_back(channel_name);
        }
    }

    if(layer_names.contains("default"))
    {
        std::sort(layer_names["default"].begin(),
                  layer_names["default"].end(),
                  [](const auto& lhs, const auto& rhs) -> bool {
                      return lhs[0] > rhs[0];
                  });

        layer_names[MediaInfo::MAIN_LAYER_NAME] = std::move(layer_names["default"]);
        layer_names.erase("default");
    }
    else
    {
        stdromano::log_error("No default layer has been found in exr file");
    }

    return layer_names;
}

bool image_read_info_exr(const stdromano::StringD& path,
                         MediaInfo& info) noexcept
{
    try
    {
        Imf::InputFile file(path.c_str());
        const Imf::Header& header = file.header();
        const Imf::ChannelList& channels = header.channels();

        Imath::Box2i data_window = file.header().dataWindow();
        Imath::Box2i display_window = file.header().displayWindow();
        float aspect_ratio = file.header().pixelAspectRatio();

        EXRLayerNames layers = image_get_layer_names_exr(channels);

        info = MediaInfo(data_window,
                         display_window,
                         aspect_ratio);

        for(const auto& exr_layer : layers)
        {
            const stdromano::Vector<stdromano::StringD>& layer_channels = exr_layer.second;

            if(layer_channels.empty())
            {
                continue;
            }

            const Imf::PixelType channel_type = channels.find(layer_channels[0].c_str()).channel().type;
            const MediaDepth depth = (channel_type == Imf::HALF) ? MediaDepth_F16 : MediaDepth_F32;

            info.create_layer(exr_layer.first.copy(),
                              static_cast<MediaFormat>(layer_channels.size() - 1),
                              depth);
        }
    }
    catch(const std::exception& e)
    {
        stdromano::log_error("Error while loading image \"{}\": {}", path, e.what());
        return false;
    }


    return true;
}

bool pixels_read_exr(const stdromano::StringD& path,
                     const stdromano::StringD& layer_name,
                     const MediaLayer& layer,
                     void* data) noexcept
{
    try
    {
        Imf::InputFile file(path.c_str());
        const Imf::Header& header = file.header();
        const Imf::ChannelList& channels = header.channels();

        EXRLayerNames layers = image_get_layer_names_exr(channels);

        auto layer_it = layers.find(layer_name);

        if(layer_it == layers.end())
        {
            return false;
        }

        const stdromano::Vector<stdromano::StringD>& layer_channels = layer_it.value();

        if(layer_channels.empty())
        {
            return false;
        }

        const std::size_t channel_size = layer.channel_size();
        const Imf::PixelType channel_type = channels.find(layer_channels[0].c_str()).channel().type;

        Imf::FrameBuffer frame_buffer;
        std::size_t offset = 0;

        for(const auto& channel : layer_channels)
        {
            frame_buffer.insert(channel.c_str(),
                                Imf::Slice(channel_type,
                                           static_cast<char*>(data) + offset,
                                           channel_size * layer_channels.size(),
                                           channel_size * layer_channels.size() * layer.parent()->get_data_width()));

            offset += channel_size;
        }

        file.setFrameBuffer(frame_buffer);
        file.readPixels(layer.parent()->data_window().min.y,
                        layer.parent()->data_window().max.y);
    }
    catch(const std::exception& e)
    {
        stdromano::log_error("Error while loading layer \"{}\" from image: \"{}\" ({})",
                             layer_name,
                             path,
                             e.what());

        return false;
    }

    return true;
}

/* Registry */

using ImageReadInfoFunc = bool(*)(const stdromano::StringD& /* path */, MediaInfo& /* info */) noexcept;
using ImageReadPixelsFunc = bool(*)(const stdromano::StringD& /* path */,
                                    const stdromano::StringD& /* layer name */,
                                    const MediaLayer& /* layer */,
                                    void* /* data */) noexcept;

static stdromano::HashMap<stdromano::StringD, ImageReadInfoFunc> g_read_funcs_table = {
    { "jpg", image_read_info_jpeg },
    { "jpeg", image_read_info_jpeg },
    { "png", image_read_info_png },
    { "hdr", image_read_info_hdr },
    { "exr", image_read_info_exr },
    { "tiff", image_read_info_tiff },
    { "tif", image_read_info_tiff },
};

static stdromano::HashMap<stdromano::StringD, ImageReadPixelsFunc> g_pix_read_funcs_table = {
    { "jpg", pixels_read_jpeg },
    { "jpeg", pixels_read_jpeg },
    { "png", pixels_read_png },
    { "hdr", pixels_read_hdr },
    { "exr", pixels_read_exr },
    { "tiff", pixels_read_tiff },
    { "tif", pixels_read_tiff },
};

/* Returns nullptr if the read function can't be found */
ImageReadInfoFunc get_image_read_info(const stdromano::StringD& ext) noexcept
{
    auto it = g_read_funcs_table.find(ext);

    return it == g_read_funcs_table.end() ? nullptr : it->second;
}

ImageReadPixelsFunc get_layer_pixels_read(const stdromano::StringD& ext) noexcept
{
    auto it = g_pix_read_funcs_table.find(ext);

    return it == g_pix_read_funcs_table.end() ? nullptr : it->second;
}

/* Generic image read static function */

bool image_read_info(const stdromano::StringD& path,
                     MediaInfo& info) noexcept
{
    const stdromano::StringD ext = path.rsplit(".");

    ImageReadInfoFunc func = get_image_read_info(ext);

    if(func == nullptr)
    {
        stdromano::log_error("No function available to load file: {} (type is: {})", path, ext);
        return false;
    }

    return func(path, info);
}

bool image_read_pixels(const stdromano::StringD& path,
                       const stdromano::StringD& layer_name,
                       void* data) noexcept
{
    const stdromano::StringD ext = path.rsplit(".");

    ImageReadPixelsFunc func = get_layer_pixels_read(ext);

    if(func == nullptr)
    {
        stdromano::log_error("No function available to load layer: {} (type is: {})", path, ext);
        return false;
    }

    return func(path, layer_name, data);
}

LOV_NAMESPACE_END
