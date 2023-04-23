// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/media.h"
#include "OpenViewer/exceptions.h"
#include "OpenViewerUtils/hash.h"
#include "OpenViewerUtils/filesystem.h"

#include "OpenImageIO/imageio.h"

LOV_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
// Single Image Media
////////////////////////////////////////////////////////////////////////////////

Image::Image(const std::string& path) 
{
    this->m_path = std::move(path);

    this->m_start = 0;
    this->m_end = 1;

    const auto input = OIIO::ImageInput::open(this->m_path);

    if(!input)
    {
        throw ImageInputError(fmt::format("[OIIO] : Cannot open input file \"{}\"\n{}", this->m_path, OIIO::geterror()));
    }

    const OIIO::ImageSpec& spec = input->spec();

    this->set_width(spec.full_width);
    this->set_height(spec.full_height);

    const uint8_t type = OIIO_TYPEDESC_TO_TYPE(spec.format);
    this->set_type(type);
    
    const bool has_alpha = spec.alpha_channel > -1;
    this->set_nchannels(spec.nchannels > 4 ? 4 : spec.nchannels < 3 ? 3 : spec.nchannels);

    input->close();

    const std::string ext = lovu::fs::get_extension(this->m_path);
    this->set_image_input_func(input_funcs[ext]);
}

Image::~Image() 
{

}

std::string Image::make_path_at_frame(const uint32_t frame) const noexcept
{
    return this->m_path;
}

uint32_t Image::get_hash_at_frame(const uint32_t frame) const noexcept
{
    return lovu::hash_string(this->m_path.c_str());
}

void Image::load_frame_to_cache(void* cache_address, const uint32_t frame) const noexcept
{
    const InputSpecs& specs = this->make_input_specs();

    this->m_input_func(cache_address, this->m_path, specs);
}

bool Image::is_cached_at_frame(const uint32_t frame) const noexcept
{
    return this->m_is_cached;
}

void Image::set_cached_at_frame(const uint32_t frame, const bool cached) noexcept
{
    this->m_is_cached = cached;
}

void Image::debug() const noexcept
{

}

////////////////////////////////////////////////////////////////////////////////
// Image Sequence Media
////////////////////////////////////////////////////////////////////////////////

ImageSequence::ImageSequence(const std::string& path) 
{
    this->m_path = std::move(path);
    
    std::smatch match;
    std::regex_search(this->m_path, match, re_frames_getter_pattern);

    if(match.size() < 3)
    {
        throw ImageInputError(fmt::format("Cannot open input file \"{}\"", this->m_path));
    }
    
    this->m_start = std::stoi(match[1].str());
    this->m_end = std::stoi(match[2].str());
    
    const std::string path_at_first_frame = this->make_path_at_frame(this->m_start);

    const auto input = OIIO::ImageInput::open(path_at_first_frame);

    if(!input)
    {
        throw ImageInputError(fmt::format("[OIIO] : Cannot open input file \"{}\"\n{}", path_at_first_frame, OIIO::geterror()));
    }

    const OIIO::ImageSpec& spec = input->spec();

    this->set_width(spec.full_width);
    this->set_height(spec.full_height);

    const uint8_t type = OIIO_TYPEDESC_TO_TYPE(spec.format);
    this->set_type(type);

    const bool has_alpha = spec.alpha_channel > -1;

    uint8_t n_channels = spec.nchannels < 3 ? spec.nchannels : 
                         spec.nchannels == 3 ? 3 :
                         spec.nchannels > 3 && has_alpha ? 4 : 3;

    this->set_nchannels(n_channels);

    input->close();

    this->m_is_cached.resize(this->get_length());

    const std::string ext = lovu::fs::get_extension(path_at_first_frame);
    this->set_image_input_func(input_funcs[&ext.c_str()[1]]);
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
    const std::string path_at_frame = this->make_path_at_frame(frame);

    const InputSpecs& specs = this->make_input_specs();
    
    this->m_input_func(cache_address, path_at_frame, specs);
}

bool ImageSequence::is_cached_at_frame(const uint32_t frame) const noexcept
{
    return this->m_is_cached.test(frame - this->m_start);
}

void ImageSequence::set_cached_at_frame(const uint32_t frame, const bool cached) noexcept
{
    if(cached) 
    {
        this->m_is_cached.set(frame - this->m_start);
    }
    else
    {
        this->m_is_cached.clear(frame - this->m_start);
    }
}

void ImageSequence::debug() const noexcept
{
    spdlog::debug("------------------------------");
    spdlog::debug("Media debugging");
    spdlog::debug("Path : {}", this->m_path);
    spdlog::debug("Frames : {}-{}", this->m_start, this->m_end);
    spdlog::debug("Width : {} px", this->get_width());
    spdlog::debug("Height : {} px", this->get_height());
    spdlog::debug("Channels : {}", this->get_nchannels());
    spdlog::debug("Depth : {} bits", this->get_type_size() * 8);
    spdlog::debug("Size : {} px", this->get_size());
    spdlog::debug("Bytes Size : {} bits", this->get_byte_size());
    
    std::string cached;
    cached.reserve(this->get_length() + 2);
    cached += "[";

    for(uint32_t i = this->m_start; i < this->m_end; i++)
    {
        if(this->is_cached_at_frame(i))
        {
            cached += "O";
        }
        else
        {
            cached += "X";
        }
    }

    cached += "]";
    
    spdlog::debug("Cached : {}", cached);
    spdlog::debug("------------------------------");
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

std::string Video::make_path_at_frame(const uint32_t frame) const noexcept
{
    return this->m_path;
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

void Video::debug() const noexcept
{

}

LOV_NAMESPACE_END
