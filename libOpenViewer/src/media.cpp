// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/media.h"
#include "OpenViewerUtils/hash.h"

LOV_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
// Single Image Media
////////////////////////////////////////////////////////////////////////////////

Image::Image(const std::string& path) 
{

}

Image::~Image() 
{

}

uint32_t Image::get_hash_at_frame(const uint32_t frame) const noexcept
{
    return EMPTY_HASH;
}

void Image::load_frame_to_cache(void* cache_address, const uint32_t frame) const noexcept
{

}

bool Image::is_cached_at_frame(const uint32_t frame) const noexcept
{
    return false;
}

void Image::set_cached_at_frame(const uint32_t frame, const bool cached) noexcept
{
    
}

////////////////////////////////////////////////////////////////////////////////
// Image Sequence Media
////////////////////////////////////////////////////////////////////////////////

ImageSequence::ImageSequence(const std::string& path) 
{

}

ImageSequence::~ImageSequence()
{

}

uint32_t ImageSequence::get_hash_at_frame(const uint32_t frame) const noexcept
{
    return EMPTY_HASH;
}

void ImageSequence::load_frame_to_cache(void* cache_address, const uint32_t frame) const noexcept
{

}

bool ImageSequence::is_cached_at_frame(const uint32_t frame) const noexcept
{
    return false;
}

void ImageSequence::set_cached_at_frame(const uint32_t frame, const bool cached) noexcept
{

}

////////////////////////////////////////////////////////////////////////////////
// Video Media
////////////////////////////////////////////////////////////////////////////////

Video::Video(const std::string& path) 
{

}

Video::~Video()
{
    
}

uint32_t Video::get_hash_at_frame(const uint32_t frame) const noexcept
{
    return EMPTY_HASH;
}

void Video::load_frame_to_cache(void* cache_address, const uint32_t frame) const noexcept
{

}

bool Video::is_cached_at_frame(const uint32_t frame) const noexcept
{
    return false;
}

void Video::set_cached_at_frame(const uint32_t frame, const bool cached) noexcept
{

}

LOV_NAMESPACE_END