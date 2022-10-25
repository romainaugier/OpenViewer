// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/media.h"
#include "OpenViewerUtils/hash.h"

LOV_NAMESPACE_BEGIN

Image::Image(const std::string& path) 
{

}

void Image::load_frame_to_cache(void* cache_address, const uint32_t frame) const noexcept
{

}

bool Image::is_cached_at_frame(const uint32_t frame) const noexcept
{

}

void Image::set_cached_at_frame(const uint32_t frame, const bool cached) noexcept
{
    
}

ImageSequence::ImageSequence(const std::string& path) 
{

}

void ImageSequence::load_frame_to_cache(void* cache_address, const uint32_t frame) const noexcept
{

}

bool ImageSequence::is_cached_at_frame(const uint32_t frame) const noexcept
{

}

void ImageSequence::set_cached_at_frame(const uint32_t frame, const bool cached) noexcept
{

}

Video::Video(const std::string& path) 
{

}

void Video::load_frame_to_cache(void* cache_address, const uint32_t frame) const noexcept
{

}

bool Video::is_cached_at_frame(const uint32_t frame) const noexcept
{

}

void Video::set_cached_at_frame(const uint32_t frame, const bool cached) noexcept
{

}

LOV_NAMESPACE_END