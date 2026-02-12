// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__LOV_MEDIA_INFO)
#define __LOV_MEDIA_INFO

#include "OpenViewer/common.hpp"

#include "stdromano/string.hpp"
#include "stdromano/optional.hpp"
#include "stdromano/hashset.hpp"

#include "Imath/ImathBox.h"

LOV_NAMESPACE_BEGIN

enum MediaFormat : std::uint8_t
{
    MediaFormat_R,
    MediaFormat_RG,
    MediaFormat_RGB,
    MediaFormat_RGBA,
};

// If the depth cannot be deduced, it will be half float by default
enum MediaDepth : std::uint8_t
{
    MediaDepth_NONE,
    MediaDepth_F16,
    MediaDepth_F32,
    MediaDepth_U8,
    MediaDepth_U16,
    MediaDepth_U32,
};

template<std::uint8_t depth>
struct depth_to_type {};

template<> struct depth_to_type<MediaDepth_U8> { using type = std::uint8_t; };
template<> struct depth_to_type<MediaDepth_U16> { using type = std::uint16_t; };
template<> struct depth_to_type<MediaDepth_U32> { using type = std::uint32_t; };
template<> struct depth_to_type<MediaDepth_F16> { using type = half; };
template<> struct depth_to_type<MediaDepth_F32> { using type = float; };

template<std::uint8_t depth>
using depth_to_type_t = typename depth_to_type<depth>::type;

template<std::uint8_t depth>
struct depth_to_byte_sz {};

template<> struct depth_to_byte_sz<MediaDepth_U8> { std::uint8_t value = sizeof(std::uint8_t); };
template<> struct depth_to_byte_sz<MediaDepth_U16> { std::uint8_t value = sizeof(std::uint16_t); };
template<> struct depth_to_byte_sz<MediaDepth_U32> { std::uint8_t value = sizeof(std::uint32_t); };
template<> struct depth_to_byte_sz<MediaDepth_F16> { std::uint8_t value = sizeof(half); };
template<> struct depth_to_byte_sz<MediaDepth_F32> { std::uint8_t value = sizeof(float); };

template<std::uint8_t depth>
using depth_to_byte_sz_v = typename depth_to_byte_sz<depth>::value;

class MediaInfo;

class MediaLayer
{
private:
    const MediaInfo* _parent;

    std::uint32_t _flags = 0;

public:
    MediaLayer(const MediaInfo* parent,
               MediaFormat format,
               MediaDepth depth) : _parent(parent)
    {
        this->_flags |= format & 0x3;
        this->_flags |= (depth & 0x7) << 2;
    }

    MediaLayer(const MediaLayer& other) : _parent(other._parent),
                                          _flags(other._flags) {}

    MediaLayer& operator=(const MediaLayer& other) noexcept
    {
        if(this != &other)
        {
            this->_parent = other._parent;
            this->_flags = other._flags;
        }

        return *this;
    }

    MediaLayer(MediaLayer&& other) : _parent(other._parent),
                                     _flags(other._flags) {}

    MediaLayer& operator=(MediaLayer&& other) noexcept
    {
        if(this != &other)
        {
            this->_parent = other._parent;
            this->_flags = other._flags;
        }

        return *this;
    }

    LOV_FORCE_INLINE const MediaInfo* parent() const noexcept { return this->_parent; }

    LOV_FORCE_INLINE MediaFormat format() const noexcept { return static_cast<MediaFormat>(this->_flags & 0x3); }

    LOV_FORCE_INLINE MediaDepth depth() const noexcept { return static_cast<MediaDepth>((this->_flags >> 2) & 0x7); }

    LOV_FORCE_INLINE std::size_t depth_size() const noexcept
    {
        switch(this->depth())
        {
            case MediaDepth_NONE:
                return 0;
            case MediaDepth_U8:
                return 1;
            case MediaDepth_U16:
            case MediaDepth_F16:
                return 2;
            case MediaDepth_F32:
            case MediaDepth_U32:
                return 4;
            default:
                return 0;
        }
    }

    // Number of channels
    LOV_FORCE_INLINE std::size_t nchannels() const noexcept { return static_cast<std::size_t>(this->format()) + 1; }

    // Depth in byte size
    LOV_FORCE_INLINE std::size_t channel_size() const noexcept { return this->depth_size(); }

    // width * height
    LOV_FORCE_INLINE std::size_t npixels() const noexcept;

    // width * height * nchannels
    LOV_FORCE_INLINE std::size_t nelements() const noexcept { return this->npixels() * this->nchannels(); }

    // width * height * nchannels * depth
    LOV_FORCE_INLINE std::size_t nbytes() const noexcept { return this->nelements() * this->depth_size(); }

    // nchannels * depth
    LOV_FORCE_INLINE std::size_t pixel_size() const noexcept { return this->depth_size() * this->nchannels(); }

    // Same as pixel_size
    LOV_FORCE_INLINE std::size_t channel_stride() const noexcept { return this->depth_size() * this->nchannels(); }
};

using Layers = stdromano::HashMap<stdromano::StringD, MediaLayer>;

class MediaInfo
{
public:
    // Equivalent to RGB, RGBA, in exr, or the only layer available if the file does not support layers
    // If the file does not support layers or only has one layer, _layers will have no value
    static constexpr const char* MAIN_LAYER_NAME = "main";

private:
    Imath::Box2i _data_window;
    Imath::Box2i _display_window;

    float _aspect_ratio = 1.0f;

    Layers _layers;
public:
    MediaInfo() = default;

    MediaInfo(Imath::Box2i data_window,
              Imath::Box2i display_window,
              float aspect_ratio) : _data_window(std::move(data_window)),
                                    _display_window(std::move(display_window)),
                                    _aspect_ratio(aspect_ratio)
    {
    }

    // ** Layers **

    const Layers& layers() const noexcept { return this->_layers; }
    Layers& layers() noexcept { return this->_layers; }

    MediaLayer create_layer(const stdromano::StringD& name,
                            MediaFormat format,
                            MediaDepth depth) noexcept
    {
        auto [it, _] = this->_layers.emplace(std::make_pair(name, MediaLayer(this,
                                                                             format,
                                                                             depth)));

       return it->second;
    }

    const MediaLayer& main() const noexcept { return this->_layers[MediaInfo::MAIN_LAYER_NAME]; }
    MediaLayer& main() noexcept { return this->_layers[MediaInfo::MAIN_LAYER_NAME]; }

    // Methods for data/display window

    LOV_FORCE_INLINE const Imath::Box2i& data_window() const noexcept { return this->_data_window; }

    LOV_FORCE_INLINE Imath::Box2i& data_window() noexcept { return this->_data_window; }

    LOV_FORCE_INLINE const Imath::Box2i& display_window() const noexcept { return this->_display_window; }

    LOV_FORCE_INLINE Imath::Box2i& display_window() noexcept { return this->_display_window; }

    LOV_FORCE_INLINE std::int32_t get_data_width() const noexcept { return this->_data_window.max.x - this->_data_window.min.x + 1; }

    LOV_FORCE_INLINE std::int32_t get_display_width() const noexcept { return this->_display_window.max.x - this->_display_window.min.x + 1; }

    LOV_FORCE_INLINE std::int32_t get_data_height() const noexcept { return this->_data_window.max.y - this->_data_window.min.y + 1; }

    LOV_FORCE_INLINE std::int32_t get_display_height() const noexcept { return this->_display_window.max.y - this->_display_window.min.y + 1; }

    LOV_FORCE_INLINE float aspect_ratio() const noexcept { return this->_aspect_ratio; }
};

LOV_FORCE_INLINE std::size_t MediaLayer::npixels() const noexcept
{
    return static_cast<std::size_t>(this->_parent->get_display_width()) *
           static_cast<std::size_t>(this->_parent->get_display_height());
}

LOV_NAMESPACE_END

#endif // #if !defined(__LOV_MEDIA_INFO)
