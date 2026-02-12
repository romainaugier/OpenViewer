// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__LOV_MEDIA)
#define __LOV_MEDIA

#include "OpenViewer/media_cache.hpp"
#include "OpenViewer/media_info.hpp"

#include "Imath/ImathBox.h"

LOV_NAMESPACE_BEGIN

// A media represents anything that OpenViewer can read, i.e a video, an image or an image sequence
// It holds informations such as the dimensions, the number of channels, the start/end frames, the pixel type
// We assume those informations will be consistent for image sequences
class LOV_API Media
{
protected:
    stdromano::StringD _path;

    std::uint32_t _start = 0;
    std::uint32_t _end = 0;

    MediaInfo _info;

public:
    // Constructs an empty media
    Media() = default;

    // Destructs a media
    virtual ~Media() = default;

    // Returns a void* to the block of data corresponding to the frame and layer (if given).
    // If the data is not available, load it in cache and then return the pointer to the data.
    virtual void* get_data_at_frame(std::uint32_t frame,
                                    MediaCache& cache,
                                    const stdromano::StringD& layer = stdromano::StringD::make_ref("main")) noexcept = 0;

    // True if the media is in the media cache
    virtual bool is_cached_at_frame(std::uint32_t frame) const noexcept = 0;

    // Debugging purpose, print the media to the console
    virtual void debug() const noexcept = 0;

    // ** Media info **
    LOV_FORCE_INLINE const MediaInfo& get_info() const noexcept { return this->_info; }

    // ** Media path **

    // Returns the path of the media
    LOV_FORCE_INLINE const stdromano::StringD& get_path() const noexcept { return this->_path; }

    // ** Media time caracteristics **

    // Returns the length of the media in frames
    LOV_FORCE_INLINE std::uint32_t get_length() const noexcept { return this->_end - this->_start; }

    LOV_FORCE_INLINE std::uint32_t get_start_frame() const noexcept { return this->_start; }

    LOV_FORCE_INLINE std::uint32_t get_end_frame() const noexcept { return this->_end; }
};

class LOV_API ImageMedia : public Media
{
public:
    ImageMedia(stdromano::StringD& path);

    ~ImageMedia() override;

    void* get_data_at_frame(std::uint32_t frame,
                            MediaCache& cache,
                            const stdromano::StringD& layer = stdromano::StringD::make_ref("main")) noexcept override;

    bool is_cached_at_frame(std::uint32_t frame) const noexcept override;

    void debug() const noexcept override;
};

class LOV_API ImageSequenceMedia : public Media
{
private:

public:
    ImageSequenceMedia(stdromano::StringD& path,
                       std::uint32_t start,
                       std::uint32_t end);

    ~ImageSequenceMedia() override;

    void* get_data_at_frame(std::uint32_t frame,
                            MediaCache& cache,
                            const stdromano::StringD& layer = stdromano::StringD::make_ref("main")) noexcept override;

    bool is_cached_at_frame(std::uint32_t frame) const noexcept override;

    void debug() const noexcept override;
};

class LOV_API VideoMedia : public Media
{
public:
    VideoMedia(stdromano::StringD& path);

    ~VideoMedia() override;

    void* get_data_at_frame(std::uint32_t frame,
                            MediaCache& cache,
                            const stdromano::StringD& layer = stdromano::StringD::make_ref("main")) noexcept override;

    bool is_cached_at_frame(std::uint32_t frame) const noexcept override;

    void debug() const noexcept override;
};

LOV_NAMESPACE_END

#endif // !defined(__LOV_MEDIA)
