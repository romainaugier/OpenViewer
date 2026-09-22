// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "readers/readers.hpp"

#include "OpenViewer/log.hpp"

#include "stdromano/vector.hpp"

#include "OpenEXR/ImfChannelList.h"
#include "OpenEXR/ImfFrameBuffer.h"
#include "OpenEXR/ImfHeader.h"
#include "OpenEXR/ImfInputFile.h"
#include "OpenEXR/ImfPixelType.h"

#include <algorithm>

LOV_NAMESPACE_BEGIN

DETAIL_NAMESPACE_BEGIN

static constexpr const char* EXR_READER_NAME = "OpenEXR";

using ExrChannels = stdromano::Vector<stdromano::StringD>;
using ExrLayers = stdromano::HashMap<stdromano::StringD, ExrChannels>;

// Rank of a channel suffix in the canonical orders. -1 when it is not part of
// the order being tested.
static int rgba_rank(const stdromano::StringD& suffix) noexcept
{
    if(suffix.size() != 1)
        return -1;

    switch(suffix[0])
    {
        case 'R':
        case 'r':
            return 0;
        case 'G':
        case 'g':
            return 1;
        case 'B':
        case 'b':
            return 2;
        case 'A':
        case 'a':
            return 3;
        default:
            return -1;
    }
}

static int xyz_rank(const stdromano::StringD& suffix) noexcept
{
    if(suffix.size() != 1)
        return -1;

    switch(suffix[0])
    {
        case 'X':
        case 'x':
            return 0;
        case 'Y':
        case 'y':
            return 1;
        case 'Z':
        case 'z':
            return 2;
        default:
            return -1;
    }
}

static stdromano::StringD channel_suffix(const stdromano::StringD& full_name) noexcept
{
    return full_name.rsplit(stdromano::StringD::make_ref("."));
}

// Puts a layer's channels in storage order

static void sort_channels(ExrChannels& channels) noexcept
{
    bool all_rgba = true;
    bool all_xyz = true;

    for(const auto& channel : channels)
    {
        const stdromano::StringD suffix = channel_suffix(channel);

        all_rgba = all_rgba && rgba_rank(suffix) >= 0;
        all_xyz = all_xyz && xyz_rank(suffix) >= 0;
    }

    if(all_rgba)
    {
        std::sort(channels.begin(),
                  channels.end(),
                  [](const stdromano::StringD& lhs, const stdromano::StringD& rhs) -> bool {
                      return rgba_rank(channel_suffix(lhs)) < rgba_rank(channel_suffix(rhs));
                  });
    }
    else if(all_xyz)
    {
        std::sort(channels.begin(),
                  channels.end(),
                  [](const stdromano::StringD& lhs, const stdromano::StringD& rhs) -> bool {
                      return xyz_rank(channel_suffix(lhs)) < xyz_rank(channel_suffix(rhs));
                  });
    }
    else
    {
        std::sort(channels.begin(),
                  channels.end(),
                  [](const stdromano::StringD& lhs, const stdromano::StringD& rhs) -> bool {
                      return std::strcmp(lhs.c_str(), rhs.c_str()) < 0;
                  });
    }
}

// Groups the file's channels into layers: channels with no dot (R, G, B, A) go to the main layer
// and "diffuse.R" goes to layer "diffuse"
static ExrLayers collect_layers(const Imf::ChannelList& channels) noexcept
{
    ExrLayers layers;

    for(Imf::ChannelList::ConstIterator it = channels.begin(); it != channels.end(); ++it)
    {
        const stdromano::StringD full_name = stdromano::StringD::make_from_c_str(it.name());

        stdromano::StringD layer_name;
        full_name.rsplit(stdromano::StringD::make_ref("."), &layer_name);

        if(layer_name.size() == 0)
            layers[stdromano::StringD::make_ref(MediaInfo::MAIN_LAYER_NAME)].push_back(full_name.copy());
        else
            layers[layer_name.copy()].push_back(full_name.copy());
    }

    for(auto& layer : layers)
        sort_channels(layer.second);

    return layers;
}

static MediaDepth depth_from_pixel_type(Imf::PixelType type) noexcept
{
    switch(type)
    {
        case Imf::HALF:
            return MediaDepth_F16;
        case Imf::FLOAT:
            return MediaDepth_F32;
        case Imf::UINT:
            return MediaDepth_U32;
        default:
            return MediaDepth_NONE;
    }
}

static bool pixel_type_from_depth(MediaDepth depth, Imf::PixelType& out) noexcept
{
    switch(depth)
    {
        case MediaDepth_F16:
            out = Imf::HALF;
            return true;
        case MediaDepth_F32:
            out = Imf::FLOAT;
            return true;
        case MediaDepth_U32:
            out = Imf::UINT;
            return true;
        default:
            return false;
    }
}

static MediaDepth layer_depth(const Imf::ChannelList& channels,
                              const ExrChannels& layer_channels) noexcept
{
    MediaDepth widest = MediaDepth_NONE;

    for(const auto& channel_name : layer_channels)
    {
        const Imf::Channel* channel = channels.findChannel(channel_name.c_str());

        if(channel == nullptr)
            continue;

        const MediaDepth depth = depth_from_pixel_type(channel->type);

        // F16 < F32 == U32 in width and we prefer F32 over U32 when mixed
        if(widest == MediaDepth_NONE || (widest == MediaDepth_F16 && depth != MediaDepth_F16))
            widest = depth;
    }

    return widest == MediaDepth_NONE ? MediaDepth_F16 : widest;
}

/* ------------------------------------------------------------------------ */

static bool exr_read_info(const stdromano::StringD& path, MediaInfo& info) noexcept
{
    try
    {
        // TODO: Multi-part exrs need Imf::MultiPartInputFile
        Imf::InputFile file(path.c_str());

        const Imf::Header& header = file.header();
        const Imf::ChannelList& channels = header.channels();

        info = MediaInfo(header.dataWindow(), header.displayWindow(), header.pixelAspectRatio());

        const ExrLayers layers = collect_layers(channels);

        for(const auto& entry : layers)
        {
            const ExrChannels& layer_channels = entry.second;

            if(layer_channels.empty())
                continue;

            MediaFormat format;

            if(!format_from_nchannels(layer_channels.size(), format))
            {
                log_warn(LogCategory::ImageReader,
                         "[{}] Skipping layer \"{}\" of \"{}\": {} channels, at most 4 supported",
                         EXR_READER_NAME,
                         entry.first,
                         path,
                         layer_channels.size());

                continue;
            }

            MediaLayer& layer = info.create_layer(entry.first,
                                                  format,
                                                  layer_depth(channels, layer_channels));

            for(const auto& channel_name : layer_channels)
                layer.add_channel(channel_name.copy());
        }

        if(info.main() == nullptr)
        {
            log_warn(LogCategory::ImageReader,
                     "[{}] \"{}\" has no unprefixed channel set, so no main layer",
                     EXR_READER_NAME,
                     path);

            return false;
        }
    }
    catch(const std::exception& e)
    {
        log_error(LogCategory::ImageReader,
                  "[{}] Failed to read info from \"{}\": {}",
                  EXR_READER_NAME,
                  path,
                  e.what());
        return false;
    }

    return true;
}

static bool exr_read_layer(const stdromano::StringD& path,
                           const stdromano::StringD& layer_name,
                           const MediaLayer& layer,
                           void* dst,
                           std::size_t dst_size) noexcept
{
    if(!check_dst_size(EXR_READER_NAME, path, layer, dst, dst_size))
        return false;

    if(layer.channels().size() != layer.nchannels())
    {
        log_error(LogCategory::ImageReader,
                  "[{}] Layer \"{}\" of \"{}\" describes {} channels but its format is {}",
                  EXR_READER_NAME,
                  layer_name,
                  path,
                  layer.channels().size(),
                  format_to_string(layer.format()));

        return false;
    }

    Imf::PixelType pixel_type;

    if(!pixel_type_from_depth(layer.depth(), pixel_type))
    {
        log_error(LogCategory::ImageReader,
                  "[{}] Depth {} cannot be read from an exr",
                  EXR_READER_NAME,
                  depth_to_string(layer.depth()));

        return false;
    }

    try
    {
        Imf::InputFile file(path.c_str());

        const Imath::Box2i& data_window = file.header().dataWindow();

        const std::uint32_t width = static_cast<std::uint32_t>(data_window.max.x - data_window.min.x + 1);
        const std::uint32_t height = static_cast<std::uint32_t>(data_window.max.y - data_window.min.y + 1);

        if(width != layer.width() || height != layer.height())
        {
            log_error(LogCategory::ImageReader,
                      "[{}] \"{}\" is {}x{} but the layer expects {}x{}",
                      EXR_READER_NAME,
                      path,
                      width,
                      height,
                      layer.width(),
                      layer.height());

            return false;
        }

        const std::size_t x_stride = layer.x_stride();
        const std::size_t y_stride = layer.y_stride();
        const std::size_t channel_size = layer.channel_size();

        char* const origin = static_cast<char*>(dst) -
                             static_cast<std::ptrdiff_t>(data_window.min.x) *
                                 static_cast<std::ptrdiff_t>(x_stride) -
                             static_cast<std::ptrdiff_t>(data_window.min.y) *
                                 static_cast<std::ptrdiff_t>(y_stride);

        Imf::FrameBuffer frame_buffer;

        for(std::size_t i = 0; i < layer.channels().size(); ++i)
        {
            const stdromano::StringD& channel_name = layer.channels()[i];

            const stdromano::StringD suffix = channel_suffix(channel_name);
            const double fill_value = rgba_rank(suffix) == 3 ? 1.0 : 0.0;

            frame_buffer.insert(channel_name.c_str(),
                                Imf::Slice(pixel_type,
                                           origin + i * channel_size,
                                           x_stride,
                                           y_stride,
                                           1,
                                           1,
                                           fill_value));
        }

        file.setFrameBuffer(frame_buffer);
        file.readPixels(data_window.min.y, data_window.max.y);
    }
    catch(const std::exception& e)
    {
        log_error(LogCategory::ImageReader,
                  "[{}] Failed to read layer \"{}\" from \"{}\": {}",
                  EXR_READER_NAME,
                  layer_name,
                  path,
                  e.what());

        return false;
    }

    return true;
}

const ImageReader g_reader_exr = {EXR_READER_NAME, exr_read_info, exr_read_layer};

DETAIL_NAMESPACE_END

LOV_NAMESPACE_END
