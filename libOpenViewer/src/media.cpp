// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/media.hpp"

#include "OpenViewer/image_reader.hpp"
#include "OpenViewer/log.hpp"

LOV_NAMESPACE_BEGIN

static constexpr int MAX_SEQUENCE_PADDING = 32;

stdromano::StringD format_sequence_path(const stdromano::StringD& pattern,
                                        std::uint32_t frame) noexcept
{
    const char* data = pattern.c_str();
    const std::size_t size = pattern.size();

    // '####'
    for(std::size_t i = 0; i < size; ++i)
    {
        if(data[i] != '#')
            continue;

        std::size_t end = i;

        while(end < size && data[end] == '#')
            ++end;

        const int padding = static_cast<int>(end - i);

        stdromano::StringD result = stdromano::StringD::make_from_c_str(data, i);
        result.appendf("{:0{}d}", frame, padding);
        result.appendc(data + end);

        return result;
    }

    // '%04d', '%4d', '%d'
    for(std::size_t i = 0; i < size; ++i)
    {
        if(data[i] != '%')
            continue;

        std::size_t end = i + 1;

        const bool zero_padded = end < size && data[end] == '0';

        if(zero_padded)
            ++end;

        int padding = 0;

        while(end < size && data[end] >= '0' && data[end] <= '9' && padding <= MAX_SEQUENCE_PADDING)
            padding = padding * 10 + (data[end++] - '0');

        if(end >= size || data[end] != 'd' || padding > MAX_SEQUENCE_PADDING)
            break;

        stdromano::StringD result = stdromano::StringD::make_from_c_str(data, i);

        if(zero_padded)
            result.appendf("{:0{}d}", frame, padding);
        else
            result.appendf("{:{}d}", frame, padding);

        result.appendc(data + end + 1);

        return result;
    }

    if(pattern.find(stdromano::StringD::make_ref("%")) >= 0)
        log_error(LogCategory::Media,
                  "Could not expand sequence pattern \"{}\": expected %d, %Nd or %0Nd",
                  pattern);

    return pattern.copy();
}

/* ------------------------------------------------------------------------ */
/* Media                                                                    */
/* ------------------------------------------------------------------------ */

bool Media::open() noexcept
{
    const stdromano::StringD path = this->frame_path(this->_start);

    if(path.size() == 0)
    {
        log_error(LogCategory::Media,
                  "Cannot resolve frame {} of \"{}\"",
                  this->_start,
                  this->_path);

        return false;
    }

    return image_read_info(path, this->_info);
}

bool Media::read_frame(std::uint32_t frame,
                       const stdromano::StringD& layer_name,
                       void* dst,
                       std::size_t dst_size) const noexcept
{
    if(!this->contains_frame(frame))
    {
        log_error(LogCategory::Media,
                  "Frame {} is outside the range [{}, {}] of \"{}\"",
                  frame,
                  this->_start,
                  this->_end,
                  this->_path);

        return false;
    }

    const MediaLayer* layer = this->_info.find_layer(layer_name);

    if(layer == nullptr)
    {
        log_error(LogCategory::Media, "\"{}\" has no layer named \"{}\"", this->_path, layer_name);
        return false;
    }

    const stdromano::StringD path = this->frame_path(frame);

    return image_read_layer(path, layer_name, *layer, dst, dst_size);
}

std::size_t Media::frame_size(const stdromano::StringD& layer_name) const noexcept
{
    const MediaLayer* layer = this->_info.find_layer(layer_name);

    return layer == nullptr ? 0 : layer->nbytes();
}

void Media::debug() const noexcept
{
    log_info(LogCategory::Media,
             "Media \"{}\" [{} - {}], {}x{} (display {}x{}), {} layer(s)",
             this->_path,
             this->_start,
             this->_end,
             this->_info.data_width(),
             this->_info.data_height(),
             this->_info.display_width(),
             this->_info.display_height(),
             this->_info.nlayers());

    for(const auto& entry : this->_info.layers())
    {
        log_info(LogCategory::Media,
                 "  layer \"{}\": {} {}, {} bytes/frame",
                 entry.first,
                 format_to_string(entry.second.format()),
                 depth_to_string(entry.second.depth()),
                 entry.second.nbytes());
    }
}

/* ------------------------------------------------------------------------ */
/* ImageMedia                                                               */
/* ------------------------------------------------------------------------ */

ImageMedia::ImageMedia(stdromano::StringD path) noexcept
{
    this->_path = std::move(path);
    this->_start = 0;
    this->_end = 0;
    this->_type = MediaType::Image;
}

stdromano::StringD ImageMedia::frame_path(std::uint32_t frame) const noexcept
{
    LOV_UNUSED(frame);

    return this->_path.copy();
}

/* ------------------------------------------------------------------------ */
/* ImageSequenceMedia                                                       */
/* ------------------------------------------------------------------------ */

ImageSequenceMedia::ImageSequenceMedia(stdromano::StringD pattern,
                                       std::uint32_t start,
                                       std::uint32_t end) noexcept
{
    this->_path = std::move(pattern);
    this->_start = start;
    this->_end = end;
    this->_type = MediaType::ImageSequence;
}

stdromano::StringD ImageSequenceMedia::frame_path(std::uint32_t frame) const noexcept
{
    if(!this->contains_frame(frame))
        return stdromano::StringD();

    return format_sequence_path(this->_path, frame);
}

/* ------------------------------------------------------------------------ */
/* VideoMedia                                                               */
/* ------------------------------------------------------------------------ */

VideoMedia::VideoMedia(stdromano::StringD path) noexcept
{
    this->_path = std::move(path);
    this->_type = MediaType::Video;
}

stdromano::StringD VideoMedia::frame_path(std::uint32_t frame) const noexcept
{
    LOV_UNUSED(frame);

    // A video has no per-frame file; the decoder seeks inside _path.
    return stdromano::StringD();
}

bool VideoMedia::read_frame(std::uint32_t frame,
                            const stdromano::StringD& layer_name,
                            void* dst,
                            std::size_t dst_size) const noexcept
{
    LOV_UNUSED(frame);
    LOV_UNUSED(layer_name);
    LOV_UNUSED(dst);
    LOV_UNUSED(dst_size);

    // TODO: libav demux + decode
    log_error(LogCategory::Media, "Video decoding is not implemented yet (\"{}\")", this->_path);

    return false;
}

bool VideoMedia::open() noexcept
{
    log_error(LogCategory::Media, "Video decoding is not implemented yet (\"{}\")", this->_path);

    return false;
}

LOV_NAMESPACE_END
