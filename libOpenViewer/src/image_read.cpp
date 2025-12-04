// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/image.hpp"

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

bool image_read_metadata_jpeg(const stdromano::StringD& path,
                              Image& img) noexcept
{
    int x, y, n;

    if(stbi_info(path.c_str(), &x, &y, &n) == 0)
    {
        stdromano::log_error("Error while loading image: \"{}\"", path);
        return false;
    }

    img.get_layers().emplace(std::make_pair(Image::MAIN_LAYER_NAME,
                                            Layer(std::addressof(img),
                                                  LayerDepth_U8,
                                                  static_cast<std::uint32_t>(n))));

    img.data_window() = Imath::Box2i(Imath::V2i(0, 0), Imath::V2i(x - 1, y - 1));
    img.display_window() = Imath::Box2i(Imath::V2i(0, 0), Imath::V2i(x - 1, y - 1));
    img.aspect_ratio() = static_cast<float>(x) / static_cast<float>(y);

    stdromano::log_debug("Loaded jpg file {} (w: {}, h:{}, l:{})",
                         stdromano::fs_filename(path),
                         img.get_data_width(),
                         img.get_data_height(),
                         img.get_layers().size());

    return true;
}

bool layer_pixel_read_jpeg(const stdromano::StringD& path,
                           const stdromano::StringD& layer_name,
                           Layer& layer) noexcept
{
    int x, y, n;

    layer.set_data(static_cast<void*>(stbi_load(path.c_str(), &x, &y, &n, 0)));

    if(layer.data<unsigned char>() == nullptr)
    {
        stdromano::log_error("Error while loading layer \"{}\" from image: \"{}\"",
                             layer_name,
                             path);

        return false;
    }

    return true;
}

/* PNG */

bool image_read_metadata_png(const stdromano::StringD& path,
                             Image& img) noexcept
{
    int x, y, n;

    if(stbi_info(path.c_str(), &x, &y, &n) == 0)
    {
        stdromano::log_error("Error while loading image: \"{}\"", path);
        return false;
    }

    img.get_layers().emplace(std::make_pair(Image::MAIN_LAYER_NAME,
                                            Layer(std::addressof(img),
                                                  LayerDepth_U8,
                                                  static_cast<std::uint32_t>(n))));

    img.data_window() = Imath::Box2i(Imath::V2i(0, 0), Imath::V2i(x - 1, y - 1));
    img.display_window() = Imath::Box2i(Imath::V2i(0, 0), Imath::V2i(x - 1, y - 1));
    img.aspect_ratio() = static_cast<float>(x) / static_cast<float>(y);

    stdromano::log_debug("Loaded png file {} (w: {}, h:{}, l:{})",
                         stdromano::fs_filename(path),
                         img.get_data_width(),
                         img.get_data_height(),
                         img.get_layers().size());

    return true;
}

bool layer_pixel_read_png(const stdromano::StringD& path,
                          const stdromano::StringD& layer_name,
                          Layer& layer) noexcept
{
    int x, y, n;

    layer.set_data(static_cast<void*>(stbi_load(path.c_str(), &x, &y, &n, 0)));

    if(layer.data<unsigned char>() == nullptr)
    {
        stdromano::log_error("Error while loading layer \"{}\" from image: \"{}\"",
                             layer_name,
                             path);

        return false;
    }

    return true;
}

/* HDR */

bool image_read_metadata_hdr(const stdromano::StringD& path,
                             Image& img) noexcept
{
    int x, y, n;

    if(stbi_info(path.c_str(), &x, &y, &n) == 0)
    {
        stdromano::log_error("Error while loading image: \"{}\"", path);
        return false;
    }

    img.get_layers().emplace(std::make_pair(Image::MAIN_LAYER_NAME,
                                            Layer(std::addressof(img),
                                                  LayerDepth_F32,
                                                  static_cast<std::uint32_t>(n))));

    img.data_window() = Imath::Box2i(Imath::V2i(0, 0), Imath::V2i(x - 1, y - 1));
    img.display_window() = Imath::Box2i(Imath::V2i(0, 0), Imath::V2i(x - 1, y - 1));
    img.aspect_ratio() = static_cast<float>(x) / static_cast<float>(y);

    stdromano::log_debug("Loaded hdr file {} (w: {}, h:{}, l:{})",
                         stdromano::fs_filename(path),
                         img.get_data_width(),
                         img.get_data_height(),
                         img.get_layers().size());

    return true;
}

bool layer_pixel_read_hdr(const stdromano::StringD& path,
                          const stdromano::StringD& layer_name,
                          Layer& layer) noexcept
{
    int x, y, n;

    layer.set_data(static_cast<void*>(stbi_loadf(path.c_str(), &x, &y, &n, 0)));

    if(layer.data<unsigned char>() == nullptr)
    {
        stdromano::log_error("Error while loading layer \"{}\" from image: \"{}\"",
                             layer_name,
                             path);
        return false;
    }

    return true;
}

/* Tiff */

void tiff_error_handler(const char* module, const char* fmt, va_list ap)
{
    char buf[1024];
    std::memset(buf, 0, sizeof(buf));

    vsnprintf(buf, 1024, fmt, ap);

    stdromano::log_error("{}", buf);
}

void tiff_warning_handler(const char* module, const char* fmt, va_list ap)
{
    char buf[1024];
    std::memset(buf, 0, sizeof(buf));

    vsnprintf(buf, 1024, fmt, ap);

    stdromano::log_warn("{}", buf);
}

bool image_read_metadata_tiff(const stdromano::StringD& path,
                              Image& img) noexcept
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

    img.data_window() = Imath::Box2i(Imath::V2i(0, 0), Imath::V2i(width, height));
    img.display_window() = Imath::Box2i(Imath::V2i(0, 0), Imath::V2i(width, height));
    img.aspect_ratio() = static_cast<float>(width) / static_cast<float>(height);

    std::uint16_t n_channels = 1, bits_per_sample = 1, sample_format = SAMPLEFORMAT_UINT;
    TIFFGetFieldDefaulted(tif, TIFFTAG_SAMPLESPERPIXEL, &n_channels);
    TIFFGetFieldDefaulted(tif, TIFFTAG_BITSPERSAMPLE, &bits_per_sample);
    TIFFGetFieldDefaulted(tif, TIFFTAG_SAMPLEFORMAT, &sample_format);

    std::uint8_t depth;

    switch(sample_format)
    {
        case SAMPLEFORMAT_UINT:
            switch(bits_per_sample)
            {
                case 8:
                    depth = LayerDepth_U8;
                    break;
                case 16:
                    depth = LayerDepth_U16;
                    break;
                case 32:
                    depth = LayerDepth_U32;
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
                    depth = LayerDepth_F16;
                    break;
                case 32:
                    depth = LayerDepth_F32;
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

    img.get_layers().emplace(std::make_pair(Image::MAIN_LAYER_NAME,
                                            Layer(std::addressof(img),
                                                  depth,
                                                  static_cast<std::uint8_t>(n_channels))));

    TIFFClose(tif);

    return true;
}

bool layer_pixel_read_tiff(const stdromano::StringD& path,
                           const stdromano::StringD& layer_name,
                           Layer& layer) noexcept
{
    TIFF* tif = TIFFOpen(path.c_str(), "r");

    layer.allocate(layer.nbytes());

    if(!TIFFReadRGBAImage(tif,
                          layer.parent()->get_data_width(),
                          layer.parent()->get_data_height(),
                          layer.data<uint32_t>(),
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

        layer_names[Image::MAIN_LAYER_NAME] = std::move(layer_names["default"]);
        layer_names.erase("default");
    }
    else
    {
        stdromano::log_error("No default layer has been found in exr file");
    }

    return layer_names;
}

bool image_read_metadata_exr(const stdromano::StringD& path,
                             Image& img) noexcept
{
    try
    {
        Imf::InputFile file(path.c_str());
        const Imf::Header& header = file.header();
        const Imf::ChannelList& channels = header.channels();

        img.data_window() = file.header().dataWindow();
        img.display_window() = file.header().displayWindow();
        img.aspect_ratio() = file.header().pixelAspectRatio();

        EXRLayerNames layers = image_get_layer_names_exr(channels);

        for(const auto& exr_layer : layers)
        {
            const stdromano::Vector<stdromano::StringD>& layer_channels = exr_layer.second;

            if(layer_channels.empty())
            {
                continue;
            }

            const Imf::PixelType channel_type = channels.find(layer_channels[0].c_str()).channel().type;
            const std::uint8_t depth = (channel_type == Imf::HALF) ? LayerDepth_F16 : LayerDepth_F32;
            const std::size_t channel_size = layer_depth_as_byte_size(depth);

            Layer layer(std::addressof(img),
                        depth,
                        static_cast<std::uint8_t>(layer_channels.size()));

            img.get_layers().emplace(std::make_pair(exr_layer.first.copy(), std::move(layer)));
        }

        stdromano::log_debug("Loaded exr file {} (w: {}, h:{}, l:{})",
                             stdromano::fs_filename(path),
                             img.get_data_width(),
                             img.get_data_height(),
                             img.get_layers().size());
    }
    catch(const std::exception& e)
    {
        stdromano::log_error("Error while loading image \"{}\": {}", path, e.what());
        return false;
    }


    return true;
}

bool layer_pixel_read_exr(const stdromano::StringD& path,
                          const stdromano::StringD& layer_name,
                          Layer& layer) noexcept
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
                                           layer.data<char>() + offset,
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

using ImageMetadataReadFunc = bool(*)(const stdromano::StringD&, Image&) noexcept;
using LayerPixelsReadFunc = bool(*)(const stdromano::StringD&,
                                    const stdromano::StringD&,
                                    Layer&) noexcept;

static stdromano::HashMap<stdromano::StringD, ImageMetadataReadFunc> g_read_funcs_table = {
    { "jpg", image_read_metadata_jpeg },
    { "jpeg", image_read_metadata_jpeg },
    { "png", image_read_metadata_png },
    { "hdr", image_read_metadata_hdr },
    { "exr", image_read_metadata_exr },
    { "tiff", image_read_metadata_tiff },
    { "tif", image_read_metadata_tiff },
};

static stdromano::HashMap<stdromano::StringD, LayerPixelsReadFunc> g_pix_read_funcs_table = {
    { "jpg", layer_pixel_read_jpeg },
    { "jpeg", layer_pixel_read_jpeg },
    { "png", layer_pixel_read_png },
    { "hdr", layer_pixel_read_hdr },
    { "exr", layer_pixel_read_exr },
    { "tiff", layer_pixel_read_tiff },
    { "tif", layer_pixel_read_tiff },
};

/* Returns nullptr if the read function can't be found */
ImageMetadataReadFunc get_image_read_metadata(const stdromano::StringD& ext) noexcept
{
    auto it = g_read_funcs_table.find(ext);

    return it == g_read_funcs_table.end() ? nullptr : it->second;
}

LayerPixelsReadFunc get_layer_pixels_read(const stdromano::StringD& ext) noexcept
{
    auto it = g_pix_read_funcs_table.find(ext);

    return it == g_pix_read_funcs_table.end() ? nullptr : it->second;
}

/* Generic image read static function */

bool Image::read_image_metadata(const stdromano::StringD& path,
                                Image& image) noexcept
{
    const stdromano::StringD ext = path.rsplit(".");

    ImageMetadataReadFunc func = get_image_read_metadata(ext);

    if(func == nullptr)
    {
        stdromano::log_error("No function available to load file: {} (type is: {})", path, ext);
        return false;
    }

    return func(path, image);
}

bool Image::read_layer_pixels(const stdromano::StringD& path,
                              const stdromano::StringD& layer_name,
                              Layer& layer) noexcept
{
    const stdromano::StringD ext = path.rsplit(".");

    LayerPixelsReadFunc func = get_layer_pixels_read(ext);

    if(func == nullptr)
    {
        stdromano::log_error("No function available to load layer: {} (type is: {})", path, ext);
        return false;
    }

    return func(path, layer_name, layer);
}

LOV_NAMESPACE_END
