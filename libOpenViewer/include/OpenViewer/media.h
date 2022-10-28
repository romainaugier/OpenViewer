// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#include "OpenViewer/openviewer.h"
#include "OpenViewer/input.h"
#include "OpenViewerUtils/bit_array.h"

#include "OpenImageIO/typedesc.h"

#include <string>
#include <string_view>
#include <filesystem>
#include <regex>
#include <optional>

LOV_NAMESPACE_BEGIN

// Formats a media can have
// All single channel media (for example, an exr file with only the R channel) will be treated as an rgb media
enum Format_
{
    Format_RGB = BIT(0),
    Format_RGBA = BIT(1)
};

// Helper macro the deduce the format given the number of channels
#define FORMAT_FROM_NCHANNELS(nchannels) nchannels <= 3 ? Format_RGB : Format_RGBA

// Type a media can have
// If the type cannot be deduced, it will be half float by default
enum Type_
{
    Type_HALF = BIT(0),
    Type_FLOAT = BIT(1),
    Type_U8 = BIT(2),
    Type_U16 = BIT(3),
    Type_U32 = BIT(4)
};

// Helper macro to convert the OIIO:TypeDesc to our internal type description
#define OIIO_TYPEDESC_TO_TYPE(t) t == OIIO::TypeDesc::FLOAT ? Type_FLOAT : t == OIIO::TypeDesc::HALF ? Type_HALF : t == OIIO::TypeDesc::UINT8 ? Type_U8 : t == OIIO::TypeDesc::UINT16 ? Type_U16 : t == OIIO::TypeDesc::UINT32 ? Type_U32 : 0

// Helper macro to get the byte size of a type
#define TYPE_BYTE_SIZE(type) (type & Type_FLOAT || type & Type_U32) ? 4 : (type & Type_HALF || type & Type_U16) ? 2 : (type & Type_U8) ? 1 : 0  

// Some image formats (exr, psd, tiff, gif) support layers, and some video formats could too (with multiple streams in one video)
// The first string represents the layer name, and the second the channels associated with it
using layer = std::pair<std::string, std::string>;

// Helper macro to get the name of a layer
#define GET_LAYER_NAME(layer) layer.first

// Helper macro to get the channels of a layer
#define GET_LAYER_CHANNELS(layer) layer.second

// Regex pattern used to replace frame numbers
static const std::basic_regex re_dash_pattern("#+");
static const std::basic_regex re_frames_pattern(" \\[.+\\]");
static const std::basic_regex re_frames_getter_pattern(" \\[(\\d+)-(\\d+)\\]");
static const std::basic_regex re_seq_pattern("^seq\\?");

// A media represents anything that OpenViewer can read, i.e a video, an image or an image sequence
// It holds informations such as the dimensions, the number of channels, the start/end frames, the pixel type
// We assume those informations will be consistent for image sequences
class LOV_DLL Media
{
public:
    // Constructs an empty media
    Media() {}

    // Destructs a media
    virtual ~Media() {}

    // Returns the hashed filename corresponding to the current frame
    virtual uint32_t get_hash_at_frame(const uint32_t frame) const noexcept = 0;

    // Loads the media content to the given cache address at the given frame
    virtual void load_frame_to_cache(void* cache_address, const uint32_t frame) const noexcept = 0;

    // Returns true if the media is cached at the given frame
    virtual bool is_cached_at_frame(const uint32_t frame) const noexcept = 0;

    // Sets the cached value to true at the given frame
    virtual void set_cached_at_frame(const uint32_t frame, const bool cached = true) noexcept = 0;

    // Debugging purpose, print the media to the console
    virtual void debug() const noexcept = 0;

    // ** Media format caracteristics **
    
    // Returns the width of the media in pixels
    LOV_FORCEINLINE uint32_t get_width() const noexcept { return this->m_width; }
    
    // Sets the width of the media in pixels
    LOV_FORCEINLINE void set_width(const uint32_t width) noexcept { this->m_width = width; }
    
    // Returns the height of the media in pixels
    LOV_FORCEINLINE uint32_t get_height() const noexcept { return this->m_height; }
    
    // Sets the width of the media in pixels
    LOV_FORCEINLINE void set_height(const uint32_t height) noexcept { this->m_height = height; }
    
    // Returns the number of channels of the media (usually 3 for rgb images, 4 for rgba images)
    LOV_FORCEINLINE uint32_t get_nchannels() const noexcept { return this->m_nchannels; }
    
    // Sets the number of channels of the media
    LOV_FORCEINLINE void set_nchannels(const uint32_t nchannels) noexcept { this->m_nchannels = nchannels; }

    // Returns the image type
    LOV_FORCEINLINE uint8_t get_type() const noexcept { return this->m_type; }

    // Returns the image type byte size
    LOV_FORCEINLINE uint8_t get_type_size() const noexcept { return TYPE_BYTE_SIZE(this->m_type); }

    // Sets the image type
    LOV_FORCEINLINE void set_type(const uint8_t type) noexcept { this->m_type = type; }

    // Returns the size of the image in pixels
    LOV_FORCEINLINE uint64_t get_size() const noexcept { return this->m_width * this->m_height * this->m_nchannels; }
    
    // Returns the size of the image in bytes
    LOV_FORCEINLINE uint64_t get_byte_size() const noexcept { return this->m_width * 
                                                                     this->m_height * 
                                                                     this->m_nchannels * 
                                                                     CAST(uint64_t, TYPE_BYTE_SIZE(this->m_type)); }
    
    // Returns the whole media byte size, i.e if its a video or a sequence, returns get_byte_size() * number of frames
    LOV_FORCEINLINE uint64_t get_media_byte_size() const noexcept { return this->get_byte_size() * 
                                                                    (this->m_end - this->m_start); }

    // ** Media path **

    // Returns the path of the media
    LOV_FORCEINLINE std::string get_path() const noexcept { return this->m_path; }
    
    // Returns the path of the media as a std::string_view
    LOV_FORCEINLINE std::string_view get_path_view() const noexcept { return this->m_path; }

    // ** Media time caracteristics **

    // Returns the length of the media in frames
    LOV_FORCEINLINE uint32_t get_length() const noexcept { return this->m_end - this->m_start; }

    LOV_FORCEINLINE uint32_t get_start_frame() const noexcept { return this->m_start; }
    
    LOV_FORCEINLINE uint32_t get_end_frame() const noexcept { return this->m_end; }

    // ** Layers **

    // Returns true if the media has layers
    LOV_FORCEINLINE bool has_layers() const noexcept { return this->m_layers.has_value(); }

    // Returns a layer given its id
    LOV_FORCEINLINE layer get_layer(const uint32_t id) const noexcept { return this->m_layers.value()[id]; }

protected:
    std::optional<std::vector<layer>> m_layers;

    std::string m_path;

    uint32_t m_start = 0;
    uint32_t m_end = 0;
private:
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    uint32_t m_nchannels = 0;

    uint16_t m_current_layer_id = 0;
    
    uint8_t m_type = 0;
};

// Single image
class LOV_DLL Image : public Media
{
public:
    Image(const std::string& path);

    virtual ~Image() override;

    virtual uint32_t get_hash_at_frame(const uint32_t frame) const noexcept override;

    virtual void load_frame_to_cache(void* cache_address, const uint32_t frame) const noexcept override;

    virtual bool is_cached_at_frame(const uint32_t frame) const noexcept override;

    virtual void set_cached_at_frame(const uint32_t frame, const bool cached = true) noexcept override;

    void set_image_input_func(image_input_func input_func) noexcept { this->m_input_func = input_func; }

    virtual void debug() const noexcept override;

private:
    bool m_is_cached : 1;

    image_input_func m_input_func = nullptr;
};

// Image sequence
// The path will be specific for a sequence, and formatted like this
// seq#D:/path/to/image_sequence_#.exr 100-150 
// There must the same amount of # as there is of padding
class LOV_DLL ImageSequence : public Media
{
public:
    ImageSequence(const std::string& path);

    virtual ~ImageSequence() override;

    std::string make_path_at_frame(const uint32_t frame) const noexcept;

    virtual uint32_t get_hash_at_frame(const uint32_t frame) const noexcept override;

    virtual void load_frame_to_cache(void* cache_address, const uint32_t frame) const noexcept override;

    virtual bool is_cached_at_frame(const uint32_t frame) const noexcept override;

    virtual void set_cached_at_frame(const uint32_t frame, const bool cached = true) noexcept override;

    void set_image_input_func(image_input_func input_func) noexcept { this->m_input_func = input_func; }

    virtual void debug() const noexcept override;

private:
    lovu::bit_array m_is_cached;

    image_input_func m_input_func = nullptr;
};

// Any video
class LOV_DLL Video : public Media
{
public:
    Video(const std::string& path);

    virtual ~Video() override;

    virtual uint32_t get_hash_at_frame(const uint32_t frame) const noexcept override;

    virtual void load_frame_to_cache(void* cache_address, const uint32_t frame) const noexcept override;

    virtual bool is_cached_at_frame(const uint32_t frame) const noexcept override;

    virtual void set_cached_at_frame(const uint32_t frame, const bool cached = true) noexcept override;

    virtual void debug() const noexcept override;

private:
    lovu::bit_array m_is_cached;
};  

// Not implemented for now
// class LOV_DLL Audio : public Media
// {
//public:
//     Audio(const std::string& path);

//     virtual ~Audio() override;

//     virtual uint32_t get_hash_at_frame(const uint32_t frame) const noexcept override;

//     virtual void load_frame_to_cache(void* cache_address, const uint32_t frame) const noexcept override;

//     virtual bool is_cached_at_frame(const uint32_t frame) const noexcept override;

//     virtual void set_cached_at_frame(const uint32_t frame, const bool cached = true) noexcept override;

// private:
//     lovu::bit_array m_is_cached;
// };

LOV_NAMESPACE_END