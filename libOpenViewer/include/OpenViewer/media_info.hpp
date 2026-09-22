// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__LOV_MEDIA_INFO)
#define __LOV_MEDIA_INFO

#include "OpenViewer/common.hpp"

#include "stdromano/hashmap.hpp"
#include "stdromano/span.hpp"
#include "stdromano/stackvector.hpp"
#include "stdromano/string.hpp"

#include "Imath/ImathBox.h"
#include "Imath/half.h"

LOV_NAMESPACE_BEGIN

/* ------------------------------------------------------------------------ */
/* Pixel layout                                                             */
/* ------------------------------------------------------------------------ */

// Pixels are always stored interleaved: R0 G0 B0 A0 R1 G1 B1 A1 ...

enum MediaFormat : std::uint8_t
{
    MediaFormat_R = 0,
    MediaFormat_RG = 1,
    MediaFormat_RGB = 2,
    MediaFormat_RGBA = 3,
};

enum MediaDepth : std::uint8_t
{
    MediaDepth_NONE = 0,
    MediaDepth_F16 = 1,
    MediaDepth_F32 = 2,
    MediaDepth_U8 = 3,
    MediaDepth_U16 = 4,
    MediaDepth_U32 = 5,
};

template <std::uint8_t depth>
struct depth_to_type
{
};

template <>
struct depth_to_type<MediaDepth_U8>
{
    using type = std::uint8_t;
};

template <>
struct depth_to_type<MediaDepth_U16>
{
    using type = std::uint16_t;
};

template <>
struct depth_to_type<MediaDepth_U32>
{
    using type = std::uint32_t;
};

template <>
struct depth_to_type<MediaDepth_F16>
{
    using type = half;
};

template <>
struct depth_to_type<MediaDepth_F32>
{
    using type = float;
};

template <std::uint8_t depth>
using depth_to_type_t = typename depth_to_type<depth>::type;

// Size in bytes of one channel sample. Returns 0 for MediaDepth_NONE
LOV_FORCE_INLINE constexpr std::size_t depth_size(MediaDepth depth) noexcept
{
    switch(depth)
    {
        case MediaDepth_U8:
            return 1;
        case MediaDepth_U16:
        case MediaDepth_F16:
            return 2;
        case MediaDepth_U32:
        case MediaDepth_F32:
            return 4;
        case MediaDepth_NONE:
        default:
            return 0;
    }
}

LOV_FORCE_INLINE constexpr const char* depth_to_string(MediaDepth depth) noexcept
{
    switch(depth)
    {
        case MediaDepth_U8:
            return "u8";
        case MediaDepth_U16:
            return "u16";
        case MediaDepth_U32:
            return "u32";
        case MediaDepth_F16:
            return "f16";
        case MediaDepth_F32:
            return "f32";
        case MediaDepth_NONE:
        default:
            return "none";
    }
}

LOV_FORCE_INLINE constexpr const char* format_to_string(MediaFormat format) noexcept
{
    switch(format)
    {
        case MediaFormat_R:
            return "R";
        case MediaFormat_RG:
            return "RG";
        case MediaFormat_RGB:
            return "RGB";
        case MediaFormat_RGBA:
            return "RGBA";
        default:
            return "unknown";
    }
}

LOV_FORCE_INLINE constexpr std::size_t format_nchannels(MediaFormat format) noexcept
{
    return static_cast<std::size_t>(format) + 1;
}

// MediaFormat has no representation for anything above 4 channels, so a layer
// with more channels than that cannot be described
LOV_FORCE_INLINE constexpr bool format_from_nchannels(std::size_t n, MediaFormat& out) noexcept
{
    if(n == 0 || n > 4)
        return false;

    out = static_cast<MediaFormat>(n - 1);

    return true;
}

/* ------------------------------------------------------------------------ */
/* MediaLayer                                                               */
/* ------------------------------------------------------------------------ */

// A single addressable image inside a media: the RGBA of a jpeg, or one AOV of a multi-layer exr
class MediaLayer
{
private:
    stdromano::StackVector<stdromano::StringD, 4> _channels;

    // Dimensions of the data window
    std::uint32_t _width = 0;
    std::uint32_t _height = 0;

    MediaFormat _format = MediaFormat_R;
    MediaDepth _depth = MediaDepth_NONE;

public:
    MediaLayer() = default;

    MediaLayer(std::uint32_t width,
               std::uint32_t height,
               MediaFormat format,
               MediaDepth depth) noexcept : _width(width),
                                            _height(height),
                                            _format(format),
                                            _depth(depth) {}

    MediaLayer(const MediaLayer&) = default;
    MediaLayer& operator=(const MediaLayer&) = default;
    MediaLayer(MediaLayer&&) noexcept = default;
    MediaLayer& operator=(MediaLayer&&) noexcept = default;

    LOV_FORCE_INLINE MediaFormat format() const noexcept { return this->_format; }

    LOV_FORCE_INLINE MediaDepth depth() const noexcept { return this->_depth; }

    LOV_FORCE_INLINE std::uint32_t width() const noexcept { return this->_width; }

    LOV_FORCE_INLINE std::uint32_t height() const noexcept { return this->_height; }

    LOV_FORCE_INLINE void set_size(std::uint32_t width, std::uint32_t height) noexcept
    {
        this->_width = width;
        this->_height = height;
    }

    // ** Channels **

    LOV_FORCE_INLINE const stdromano::Span<const stdromano::StringD> channels() const noexcept
    {
        return stdromano::make_cspan(this->_channels);
    }

    LOV_FORCE_INLINE void add_channel(stdromano::StringD name) noexcept
    {
        this->_channels.push_back(std::move(name));
    }

    // ** Sizes **

    LOV_FORCE_INLINE std::size_t nchannels() const noexcept
    {
        return format_nchannels(this->_format);
    }

    LOV_FORCE_INLINE std::size_t channel_size() const noexcept
    {
        return depth_size(this->_depth);
    }

    // Bytes for one pixel, all channels
    LOV_FORCE_INLINE std::size_t pixel_size() const noexcept
    {
        return this->channel_size() * this->nchannels();
    }

    // Distance in bytes between two horizontally adjacent pixels
    LOV_FORCE_INLINE std::size_t x_stride() const noexcept { return this->pixel_size(); }

    // Distance in bytes between two vertically adjacent pixels. No row padding
    LOV_FORCE_INLINE std::size_t y_stride() const noexcept
    {
        return this->pixel_size() * static_cast<std::size_t>(this->_width);
    }

    LOV_FORCE_INLINE std::size_t npixels() const noexcept
    {
        return static_cast<std::size_t>(this->_width) * static_cast<std::size_t>(this->_height);
    }

    LOV_FORCE_INLINE std::size_t nelements() const noexcept
    {
        return this->npixels() * this->nchannels();
    }

    // Total byte size of a buffer holding this layer for one frame. This is the number the cache allocates
    LOV_FORCE_INLINE std::size_t nbytes() const noexcept
    {
        return this->nelements() * this->channel_size();
    }

    LOV_FORCE_INLINE bool is_valid() const noexcept
    {
        return this->_depth != MediaDepth_NONE && this->_width > 0 && this->_height > 0;
    }
};

using Layers = stdromano::HashMap<stdromano::StringD, MediaLayer>;

/* ------------------------------------------------------------------------ */
/* MediaInfo                                                                */
/* ------------------------------------------------------------------------ */

class MediaInfo
{
public:
    // The layer a viewer shows by default: the RGB(A) of an exr, or the only layer of a format that has no concept of layers (jpg, png...)
    static constexpr const char* MAIN_LAYER_NAME = "main";

private:
    Imath::Box2i _data_window;
    Imath::Box2i _display_window;

    float _aspect_ratio = 1.0f;

    Layers _layers;

public:
    MediaInfo() = default;

    MediaInfo(const Imath::Box2i& data_window,
              const Imath::Box2i& display_window,
              float aspect_ratio) noexcept : _data_window(data_window),
                                             _display_window(display_window),
                                             _aspect_ratio(aspect_ratio) {}

    static MediaInfo from_size(std::uint32_t width, std::uint32_t height) noexcept
    {
        const Imath::Box2i window(Imath::V2i(0, 0),
                                  Imath::V2i(static_cast<int>(width) - 1,
                                             static_cast<int>(height) - 1));

        return MediaInfo(window, window, 1.0f);
    }

    // ** Windows **

    LOV_FORCE_INLINE const Imath::Box2i& data_window() const noexcept
    {
        return this->_data_window;
    }

    LOV_FORCE_INLINE const Imath::Box2i& display_window() const noexcept
    {
        return this->_display_window;
    }

    void set_windows(const Imath::Box2i& data_window,
                     const Imath::Box2i& display_window) noexcept
    {
        this->_data_window = data_window;
        this->_display_window = display_window;

        for(auto& layer : this->_layers)
            layer.second.set_size(this->data_width(), this->data_height());
    }

    LOV_FORCE_INLINE std::uint32_t data_width() const noexcept
    {
        return static_cast<std::uint32_t>(this->_data_window.max.x - this->_data_window.min.x + 1);
    }

    LOV_FORCE_INLINE std::uint32_t data_height() const noexcept
    {
        return static_cast<std::uint32_t>(this->_data_window.max.y - this->_data_window.min.y + 1);
    }

    LOV_FORCE_INLINE std::uint32_t display_width() const noexcept
    {
        return static_cast<std::uint32_t>(this->_display_window.max.x -
                                          this->_display_window.min.x + 1);
    }

    LOV_FORCE_INLINE std::uint32_t display_height() const noexcept
    {
        return static_cast<std::uint32_t>(this->_display_window.max.y -
                                          this->_display_window.min.y + 1);
    }

    LOV_FORCE_INLINE bool has_overscan() const noexcept
    {
        return this->_data_window != this->_display_window;
    }

    LOV_FORCE_INLINE float aspect_ratio() const noexcept { return this->_aspect_ratio; }

    LOV_FORCE_INLINE void set_aspect_ratio(float ratio) noexcept
    {
        this->_aspect_ratio = ratio;
    }

    // ** Layers **

    LOV_FORCE_INLINE const Layers& layers() const noexcept { return this->_layers; }

    LOV_FORCE_INLINE Layers& layers() noexcept { return this->_layers; }

    LOV_FORCE_INLINE std::size_t nlayers() const noexcept { return this->_layers.size(); }

    MediaLayer& create_layer(const stdromano::StringD& name,
                             MediaFormat format,
                             MediaDepth depth) noexcept
    {
        auto res = this->_layers.emplace(std::make_pair(name.copy(),
                                                        MediaLayer(this->data_width(),
                                                                   this->data_height(),
                                                                   format,
                                                                   depth)));

        return res.first->second;
    }

    const MediaLayer* find_layer(const stdromano::StringD& name) const noexcept
    {
        auto it = this->_layers.find(name);

        return it == this->_layers.end() ? nullptr : std::addressof(it->second);
    }

    MediaLayer* find_layer(const stdromano::StringD& name) noexcept
    {
        auto it = this->_layers.find(name);

        return it == this->_layers.end() ? nullptr : std::addressof(it->second);
    }

    LOV_FORCE_INLINE const MediaLayer* main() const noexcept
    {
        return this->find_layer(stdromano::StringD::make_ref(MediaInfo::MAIN_LAYER_NAME));
    }

    LOV_FORCE_INLINE MediaLayer* main() noexcept
    {
        return this->find_layer(stdromano::StringD::make_ref(MediaInfo::MAIN_LAYER_NAME));
    }

    bool is_valid() const noexcept
    {
        if(this->_data_window.isEmpty() || this->_layers.empty())
            return false;

        const MediaLayer* main_layer = this->main();

        return main_layer != nullptr && main_layer->is_valid();
    }
};

LOV_NAMESPACE_END

#endif // !defined(__LOV_MEDIA_INFO)
