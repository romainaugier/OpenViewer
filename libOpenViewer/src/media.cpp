// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/media.hpp"

LOV_NAMESPACE_BEGIN

// Image

ImageMedia::ImageMedia(stdromano::StringD& path)
{
    this->_path = std::move(path);
    this->_start = 0;
    this->_end = 0;
}

ImageMedia::~ImageMedia()
{

}

void* ImageMedia::get_data_at_frame(std::uint32_t frame,
                                    MediaCache& cache,
                                    const stdromano::StringD& layer) noexcept
{
    return nullptr;
}

bool ImageMedia::is_cached_at_frame(std::uint32_t frame) const noexcept
{
    return false;
}

void ImageMedia::debug() const noexcept
{

}

// Image Sequence

ImageSequenceMedia::ImageSequenceMedia(stdromano::StringD& path,
                                       std::uint32_t start,
                                       std::uint32_t end)
{
    this->_path = std::move(path);
    this->_start = start;
    this->_end = end;
}

ImageSequenceMedia::~ImageSequenceMedia()
{

}

void* ImageSequenceMedia::get_data_at_frame(std::uint32_t frame,
                                            MediaCache& cache,
                                            const stdromano::StringD& layer) noexcept
{
    return nullptr;
}

bool ImageSequenceMedia::is_cached_at_frame(std::uint32_t frame) const noexcept
{
    return false;
}

void ImageSequenceMedia::debug() const noexcept
{

}

// Video

VideoMedia::VideoMedia(stdromano::StringD& path)
{

}

VideoMedia::~VideoMedia()
{

}

void* VideoMedia::get_data_at_frame(std::uint32_t frame,
                                    MediaCache& cache,
                                    const stdromano::StringD& layer) noexcept
{
    return nullptr;
}

bool VideoMedia::is_cached_at_frame(std::uint32_t frame) const noexcept
{
    return false;
}

void VideoMedia::debug() const noexcept
{

}

LOV_NAMESPACE_END
