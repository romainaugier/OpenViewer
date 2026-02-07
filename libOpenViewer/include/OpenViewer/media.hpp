// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__LOV_MEDIA)
#define __LOV_MEDIA

#include "OpenViewer/common.hpp"

#include "stdromano/string.hpp"
#include "stdromano/hashset.hpp"
#include "stdromano/optional.hpp"

#include "Imath/ImathBox.h"

LOV_NAMESPACE_BEGIN

// All single channel media (for example, an exr file with only the R channel) will be treated as an rgb media
enum MediaFormat
{
    MediaFormat_RGB,
    MediaFormat_RGBA,
};

// Helper macro the deduce the format given the number of channels
#define FORMAT_FROM_NCHANNELS(nchannels) nchannels <= 3 ? Format_RGB : Format_RGBA

// If the type cannot be deduced, it will be half float by default
enum MediaDepth
{
    MediaDepth_HALF,
    MediaDepth_FLOAT,
    MediaDepth_U8,
    MediaDepth_U16,
    MediaDepth_U32,
};

struct MediaFlags
{
    std::uint64_t format : 1;
    std::uint64_t depth : 3;
};

// A media represents anything that OpenViewer can read, i.e a video, an image or an image sequence
// It holds informations such as the dimensions, the number of channels, the start/end frames, the pixel type
// We assume those informations will be consistent for image sequences
class LOV_API Media
{
public:
    // Equivalent to RGB, RGBA, in exr, or the only layer available if the file does not support layers
    // If the file does not support layers or only has one layer, _layers will have no value
    static constexpr const char* MAIN_LAYER_NAME = "main";

protected:
    stdromano::StringD _path;

    stdromano::Optional<stdromano::HashSet<stdromano::StringD>> _layers;

    uint32_t _start = 0;
    uint32_t _end = 0;

    Imath::Box2i _data_window;
    Imath::Box2i _display_window;

    float _aspect_ratio;

    uint8_t _nchannels = 0;

    MediaFlags _flags;

public:
    // Constructs an empty media
    Media() = default;

    // Destructs a media
    virtual ~Media() = default;

    //
    virtual void* get_data_at_frame(std::uint32_t frame,
                                    const stdromano::StringD& layer = stdromano::StringD::make_ref("main")) noexcept = 0;

    // True if the media is in the media cache
    virtual bool is_cached_at_frame(std::uint32_t frame) const noexcept = 0;

    // Debugging purpose, print the media to the console
    virtual void debug() const noexcept = 0;

    // ** Media format caracteristics **

    // ** Media path **

    // Returns the path of the media
    LOV_FORCE_INLINE const stdromano::StringD& get_path() const noexcept { return this->_path; }

    // ** Media time caracteristics **

    // Returns the length of the media in frames
    LOV_FORCE_INLINE std::uint32_t get_length() const noexcept { return this->_end - this->_start; }

    LOV_FORCE_INLINE std::uint32_t get_start_frame() const noexcept { return this->_start; }

    LOV_FORCE_INLINE std::uint32_t get_end_frame() const noexcept { return this->_end; }

    // ** Layers **

    // Returns true if the media has layers
    LOV_FORCE_INLINE bool has_layers() const noexcept { return this->_layers.has_value(); }

    // Returns true if the media has this layer
    LOV_FORCE_INLINE bool has_layer(const stdromano::StringD& name) const noexcept { return this->_layers.has_value() && this->_layers.value().contains(name); }
};

LOV_NAMESPACE_END

#endif // !defined(__LOV_MEDIA)
