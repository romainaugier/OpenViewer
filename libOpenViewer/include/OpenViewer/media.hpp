// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__LOV_MEDIA)
#define __LOV_MEDIA

#include "OpenViewer/media_info.hpp"

LOV_NAMESPACE_BEGIN

enum class MediaType : std::uint8_t
{
    Image,
    ImageSequence,
    Video,
};

// Anything OpenViewer can play: a still, an image sequence, or a video.
//
// A Media knows what it is, where its frames live, and what they look like. It
// does not know about the cache, and read_frame() writes into memory the caller
// owns. The previous interface took a MediaCache& on every read, which meant
// every media implementation had to understand eviction, every test had to
// build a cache, and the cache could not be changed without touching every
// reader. The scheduler above owns that relationship now.
class LOV_API Media
{
protected:
    // For a sequence this is the pattern ("render.####.exr"), not a real file.
    stdromano::StringD _path;

    // Inclusive frame range: a single image is [0, 0].
    std::uint32_t _start = 0;
    std::uint32_t _end = 0;

    MediaInfo _info;

    MediaType _type = MediaType::Image;

public:
    Media() = default;

    virtual ~Media() = default;

    // Path of the file backing `frame`. Empty when the frame is out of range or
    // when the media has no per-frame file (video).
    virtual stdromano::StringD frame_path(std::uint32_t frame) const noexcept = 0;

    // Decodes one layer of one frame into `dst`. Re-entrant: several worker
    // threads may call it on the same Media at the same time. Returns false and
    // logs on failure.
    virtual bool read_frame(std::uint32_t frame,
                            const stdromano::StringD& layer_name,
                            void* dst,
                            std::size_t dst_size) const noexcept;

    // Byte size one frame of the given layer needs. 0 when the layer is unknown.
    std::size_t frame_size(const stdromano::StringD& layer_name) const noexcept;

    // Populates _info from the first frame. Must be called once after
    // construction; returns false when the media cannot be opened.
    virtual bool open() noexcept;

    void debug() const noexcept;

    LOV_FORCE_INLINE MediaType type() const noexcept { return this->_type; }

    LOV_FORCE_INLINE const MediaInfo& info() const noexcept { return this->_info; }

    LOV_FORCE_INLINE const stdromano::StringD& path() const noexcept { return this->_path; }

    LOV_FORCE_INLINE std::uint32_t start_frame() const noexcept { return this->_start; }

    LOV_FORCE_INLINE std::uint32_t end_frame() const noexcept { return this->_end; }

    // Inclusive range, so a single image has a length of 1.
    LOV_FORCE_INLINE std::uint32_t length() const noexcept
    {
        return this->_end - this->_start + 1;
    }

    LOV_FORCE_INLINE bool contains_frame(std::uint32_t frame) const noexcept
    {
        return frame >= this->_start && frame <= this->_end;
    }
};

class LOV_API ImageMedia : public Media
{
public:
    explicit ImageMedia(stdromano::StringD path) noexcept;

    stdromano::StringD frame_path(std::uint32_t frame) const noexcept override;
};

class LOV_API ImageSequenceMedia : public Media
{
public:
    // `pattern` holds either a run of '#' (render.####.exr) or a printf
    // conversion (render.%04d.exr).
    ImageSequenceMedia(stdromano::StringD pattern,
                       std::uint32_t start,
                       std::uint32_t end) noexcept;

    stdromano::StringD frame_path(std::uint32_t frame) const noexcept override;
};

class LOV_API VideoMedia : public Media
{
public:
    explicit VideoMedia(stdromano::StringD path) noexcept;

    stdromano::StringD frame_path(std::uint32_t frame) const noexcept override;

    bool read_frame(std::uint32_t frame,
                    const stdromano::StringD& layer_name,
                    void* dst,
                    std::size_t dst_size) const noexcept override;

    bool open() noexcept override;
};

// Substitutes `frame` into a sequence pattern. Supports '####' and '%04d'.
// Returns a copy of the pattern when it holds neither.
LOV_API stdromano::StringD format_sequence_path(const stdromano::StringD& pattern,
                                                std::uint32_t frame) noexcept;

LOV_NAMESPACE_END

#endif // !defined(__LOV_MEDIA)
