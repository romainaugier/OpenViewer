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
    this->m_path = path;
}

Image::~Image() 
{

}

uint32_t Image::get_hash_at_frame(const uint32_t frame) const noexcept
{
    return lovu::hash_string(this->m_path.c_str());
}

void Image::load_frame_to_cache(void* cache_address, const uint32_t frame) const noexcept
{

}

bool Image::is_cached_at_frame(const uint32_t frame) const noexcept
{
    return this->m_is_cached;
}

void Image::set_cached_at_frame(const uint32_t frame, const bool cached) noexcept
{
    this->m_is_cached = cached;
}

////////////////////////////////////////////////////////////////////////////////
// Image Sequence Media
////////////////////////////////////////////////////////////////////////////////

ImageSequence::ImageSequence(const std::string& path) 
{
    this->m_path = path;
}

ImageSequence::~ImageSequence()
{

}

std::string ImageSequence::make_path_at_frame(const uint32_t frame) const noexcept
{
    std::smatch match;
    std::regex_search(this->m_path, match, re_dash_pattern);

    const uint8_t padding = match.length();

    std::string tmp_path = std::regex_replace(this->m_path, re_frames_pattern, "");

    tmp_path = std::regex_replace(tmp_path, re_seq_pattern, "");

    return std::regex_replace(tmp_path, re_dash_pattern, fmt::format("{0:0{1}}", frame, padding));

}

uint32_t ImageSequence::get_hash_at_frame(const uint32_t frame) const noexcept
{
    const std::string path_at_frame = this->make_path_at_frame(frame);
    
    return lovu::hash_string(path_at_frame.c_str());
}

void ImageSequence::load_frame_to_cache(void* cache_address, const uint32_t frame) const noexcept
{

}

bool ImageSequence::is_cached_at_frame(const uint32_t frame) const noexcept
{
    return this->m_is_cached.test(frame);
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