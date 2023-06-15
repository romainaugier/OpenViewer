// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/media.h"
#include "OpenViewer/exceptions.h"
#include "OpenViewerUtils/hash.h"
#include "OpenViewerUtils/filesystem.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "OpenEXR/ImfInputFile.h"
#include "OpenEXR/ImfHeader.h"
#include "OpenEXR/ImfChannelList.h"
#include "Imath/ImathBox.h"

LOV_NAMESPACE_BEGIN

int get_image_infos(const char* path,
                    int* width,
                    int* height,
                    int* channels,
                    Type_* type) noexcept
{
    const std::string ext = lovu::fs::get_extension(path);

    if(ext == "exr")
    {
        Imf::InputFile in(path);

        const Imath::Box2i data_window = in.header().dataWindow();

        const Imf::ChannelList &exr_channels = in.header().channels();

        if(exr_channels.findChannel("R") && exr_channels.findChannel("G") && exr_channels.findChannel("B"))
        {
            *channels = 3;
        }

        if(exr_channels.findChannel("A"))
        {
            *channels += 1;
        }

        *type = in.header().channels()["R"].type == Imf::HALF ? Type_HALF : Type_FLOAT;

        *width = data_window.max.x - data_window.min.x + 1;
        *height = data_window.max.y - data_window.min.y + 1;
    }
    else
    {
        FILE* f = fopen(path, "rb");

        if(!f)
        {
            return 0;
        }

        if(!stbi_info_from_file(f, width, height, channels))
        {
            return 0;
        }

        if(ext == "jpg") *type = Type_U8;
        else if(ext == "png") *type = stbi_is_16_bit_from_file(f) == 1 ? Type_U16 : Type_U8;
        else *type = Type_U8;
    }

    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Single Image Media
////////////////////////////////////////////////////////////////////////////////

Image::Image(const std::string path) 
{
    this->m_path = std::move(path);

    this->m_start = 0;
    this->m_end = 1;

    int width, height, channels;
    Type_ type;

    get_image_infos(this->m_path.c_str(), &width, &height, &channels, &type);
    
    this->set_width(width);
    this->set_height(height);
    this->set_nchannels(channels);
    this->set_type(type);

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
    assert(this->m_input_func != nullptr);

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

ImageSequence::ImageSequence(const std::string path) 
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

    int width, height, channels;
    Type_ type;

    get_image_infos(path_at_first_frame.c_str(), &width, &height, &channels, &type);
    
    this->set_width(width);
    this->set_height(height);
    this->set_nchannels(channels);
    this->set_type(type);

    this->m_is_cached.resize(this->get_length());

    const std::string ext = lovu::fs::get_extension(path_at_first_frame);
    this->set_image_input_func(input_funcs[ext]);
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
