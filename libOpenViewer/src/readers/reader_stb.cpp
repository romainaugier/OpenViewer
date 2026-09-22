// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "readers/readers.hpp"

#include "OpenViewer/log.hpp"

#include "stdromano/memory.hpp"
#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_MALLOC stdromano::mem_alloc
#define STBI_REALLOC stdromano::mem_realloc
#define STBI_FREE stdromano::mem_free
#include "stb_image_read.hpp"

LOV_NAMESPACE_BEGIN

DETAIL_NAMESPACE_BEGIN

static constexpr const char* STB_LDR_READER_NAME = "stb (ldr)";
static constexpr const char* STB_HDR_READER_NAME = "stb (hdr)";

// TODO: stb always allocates its own buffer, so every read costs one full
// frame memcpy on top of the decode, and stb's jpeg decoder is roughly 2-3x
// slower than libjpeg-turbo

static bool stb_read_info_impl(const char* reader_name,
                               const stdromano::StringD& path,
                               MediaDepth depth,
                               MediaInfo& info) noexcept
{
    int width = 0;
    int height = 0;
    int nchannels = 0;

    if(stbi_info(path.c_str(), &width, &height, &nchannels) == 0)
    {
        log_error(LogCategory::ImageReader,
                  "[{}] Cannot read \"{}\": {}",
                  reader_name,
                  path,
                  stbi_failure_reason());
        return false;
    }

    MediaFormat format;

    if(!format_from_nchannels(static_cast<std::size_t>(nchannels), format))
    {
        log_error(LogCategory::ImageReader,
                  "[{}] \"{}\" has {} channels, at most 4 supported",
                  reader_name,
                  path,
                  nchannels);

        return false;
    }

    info = MediaInfo::from_size(static_cast<std::uint32_t>(width),
                                static_cast<std::uint32_t>(height));

    MediaLayer& layer = info.create_layer(stdromano::StringD::make_ref(MediaInfo::MAIN_LAYER_NAME),
                                          format,
                                          depth);

    static const char* CHANNEL_NAMES[] = {"R", "G", "B", "A"};

    for(int i = 0; i < nchannels; ++i)
        layer.add_channel(stdromano::StringD::make_from_c_str(CHANNEL_NAMES[i]));

    return true;
}

static bool stb_ldr_read_info(const stdromano::StringD& path, MediaInfo& info) noexcept
{
    return stb_read_info_impl(STB_LDR_READER_NAME, path, MediaDepth_U8, info);
}

static bool stb_hdr_read_info(const stdromano::StringD& path, MediaInfo& info) noexcept
{
    return stb_read_info_impl(STB_HDR_READER_NAME, path, MediaDepth_F32, info);
}

static bool stb_ldr_read_layer(const stdromano::StringD& path,
                               const stdromano::StringD& layer_name,
                               const MediaLayer& layer,
                               void* dst,
                               std::size_t dst_size) noexcept
{
    LOV_UNUSED(layer_name);

    if(!check_dst_size(STB_LDR_READER_NAME, path, layer, dst, dst_size))
        return false;

    int width = 0;
    int height = 0;
    int file_channels = 0;

    stbi_uc* pixels = stbi_load(path.c_str(),
                                &width,
                                &height,
                                &file_channels,
                                static_cast<int>(layer.nchannels()));

    if(pixels == nullptr)
    {
        log_error(LogCategory::ImageReader,
                  "[{}] Cannot decode \"{}\": {}",
                  STB_LDR_READER_NAME,
                  path,
                  stbi_failure_reason());

        return false;
    }

    if(static_cast<std::uint32_t>(width) != layer.width() ||
       static_cast<std::uint32_t>(height) != layer.height())
    {
        log_error(LogCategory::ImageReader,
                  "[{}] \"{}\" is {}x{} but the layer expects {}x{}",
                  STB_LDR_READER_NAME,
                  path,
                  width,
                  height,
                  layer.width(),
                  layer.height());

        stbi_image_free(pixels);

        return false;
    }

    std::memcpy(dst, pixels, layer.nbytes());
    stbi_image_free(pixels);

    return true;
}

static bool stb_hdr_read_layer(const stdromano::StringD& path,
                               const stdromano::StringD& layer_name,
                               const MediaLayer& layer,
                               void* dst,
                               std::size_t dst_size) noexcept
{
    LOV_UNUSED(layer_name);

    if(!check_dst_size(STB_HDR_READER_NAME, path, layer, dst, dst_size))
        return false;

    int width = 0;
    int height = 0;
    int file_channels = 0;

    float* pixels = stbi_loadf(path.c_str(),
                               &width,
                               &height,
                               &file_channels,
                               static_cast<int>(layer.nchannels()));

    if(pixels == nullptr)
    {
        log_error(LogCategory::ImageReader,
                  "[{}] Cannot decode \"{}\": {}",
                  STB_HDR_READER_NAME,
                  path,
                  stbi_failure_reason());

        return false;
    }

    if(static_cast<std::uint32_t>(width) != layer.width() ||
       static_cast<std::uint32_t>(height) != layer.height())
    {
        log_error(LogCategory::ImageReader,
                  "[{}] \"{}\" is {}x{} but the layer expects {}x{}",
                  STB_HDR_READER_NAME,
                  path,
                  width,
                  height,
                  layer.width(),
                  layer.height());
        stbi_image_free(pixels);

        return false;
    }

    std::memcpy(dst, pixels, layer.nbytes());
    stbi_image_free(pixels);

    return true;
}

const ImageReader g_reader_stb_ldr = { STB_LDR_READER_NAME, stb_ldr_read_info, stb_ldr_read_layer };

const ImageReader g_reader_stb_hdr = { STB_HDR_READER_NAME, stb_hdr_read_info, stb_hdr_read_layer };

DETAIL_NAMESPACE_END

LOV_NAMESPACE_END
