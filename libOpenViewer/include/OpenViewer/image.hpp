// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__LOV_IMAGE)
#define __LOV_IMAGE

#include "OpenViewer/common.hpp"

#include "stdromano/string.hpp"
#include "stdromano/hashmap.hpp"

#include "Imath/ImathBox.h"
#include "Imath/half.h"

LOV_NAMESPACE_BEGIN

enum LayerDepth_ : std::uint8_t
{
    LayerDepth_NONE,
    LayerDepth_U8,
    LayerDepth_U16,
    LayerDepth_U32,
    LayerDepth_F8,
    LayerDepth_F16,
    LayerDepth_F32,
};

template<std::uint8_t depth>
struct depth_to_type {};

template<> struct depth_to_type<LayerDepth_U8> { using type = std::uint8_t; };
template<> struct depth_to_type<LayerDepth_U16> { using type = std::uint16_t; };
template<> struct depth_to_type<LayerDepth_U32> { using type = std::uint32_t; };
template<> struct depth_to_type<LayerDepth_F16> { using type = half; };
template<> struct depth_to_type<LayerDepth_F32> { using type = float; };

template<std::uint8_t depth>
using depth_to_type_t = typename depth_to_type<depth>::type;

LOV_API std::size_t layer_depth_as_byte_size(std::uint8_t layer_depth) noexcept;

enum ResizeMode_ : std::uint8_t
{
    ResizeMode_BiLinear,
    ResizeMode_BiCubic,
};

class Image;

class LOV_API Layer
{
public:
    friend class Image;

private:
    static constexpr std::size_t ALIGNMENT = 32;

    const Image* _parent;

    void* _data;

    std::uint8_t _depth;
    std::uint8_t _nchannels;

    void resize(const Imath::Box2i& new_window,
                std::uint32_t mode = ResizeMode_BiCubic) noexcept;

    void crop(const Imath::Box2i& new_window) noexcept;

public:
    Layer(const Image* parent) : _parent(parent),
                                 _data(nullptr),
                                 _depth(LayerDepth_NONE),
                                 _nchannels(0) {}

    Layer(const Image* parent,
          void* data,
          std::uint8_t depth,
          std::uint8_t nchannels) : _parent(parent),
                                    _data(data),
                                    _depth(depth),
                                    _nchannels(nchannels) {}

    Layer(const Image* parent,
          std::uint8_t depth,
          std::uint8_t nchannels) : _parent(parent),
                                    _data(nullptr),
                                    _depth(depth),
                                    _nchannels(nchannels) {}

    ~Layer() noexcept;

    Layer(const Layer& other);
    Layer& operator=(const Layer& other) noexcept;

    Layer(Layer&& other) noexcept;
    Layer& operator=(Layer&& other) noexcept;

    const Image* parent() const noexcept
    {
#if defined(LOV_PARANOID)
        LOV_ASSERT(this->_parent != nullptr, "Parent is nullptr");
#endif /* defined(LOV_PARANOID) */

        return this->_parent;
    }

    LOV_FORCE_INLINE bool has_data() const noexcept
    {
        return this->_data != nullptr;
    }

    template<typename T>
    LOV_FORCE_INLINE const T* data() const noexcept
    {
        static_assert(std::is_arithmetic_v<T> ||
                      std::is_same_v<T, half> ||
                      std::is_same_v<T, void>,
                      "T must be an arithmetic or half or void type");

#if defined(LOV_PARANOID)
        LOV_ASSERT(this->_data != nullptr, "Data is nullptr, Layer has not been loaded");
#endif /* defined(LOV_PARANOID) */

        return static_cast<const T*>(this->_data);
    }

    template<typename T>
    LOV_FORCE_INLINE T* data() noexcept
    {
        static_assert(std::is_arithmetic_v<T> ||
                      std::is_same_v<T, half> ||
                      std::is_same_v<T, void>,
                      "T must be an arithmetic or half or void type");

#if defined(LOV_PARANOID)
        LOV_ASSERT(this->_data != nullptr, "Data is nullptr, Layer has not been loaded");
#endif /* defined(LOV_PARANOID) */

        return static_cast<T*>(this->_data);
    }

    LOV_FORCE_INLINE void set_data(void* data) noexcept
    {
        this->_data = data;
    }

    LOV_FORCE_INLINE std::uint8_t depth() const noexcept
    {
        return this->_depth;
    }

    LOV_FORCE_INLINE std::uint8_t nchannels() const noexcept
    {
        return this->_nchannels;
    }

    /* width * height */
    LOV_FORCE_INLINE std::size_t npixels() const noexcept;

    /* width * height * nchannels */
    LOV_FORCE_INLINE std::size_t nelements() const noexcept
    {
        return this->npixels() * this->nchannels();
    }

    /* width * height * nchannels * depth */
    LOV_FORCE_INLINE std::size_t nbytes() const noexcept
    {
        return this->nelements() * layer_depth_as_byte_size(this->_depth);
    }

    /* nchannels * depth */
    LOV_FORCE_INLINE std::size_t pixel_size() const noexcept
    {
        return layer_depth_as_byte_size(this->_depth) * this->_nchannels;
    }

    /* depth */
    LOV_FORCE_INLINE std::size_t channel_size() const noexcept
    {
        return layer_depth_as_byte_size(this->_depth);
    }

    /* same as pixel_size */
    LOV_FORCE_INLINE std::size_t channel_stride() const noexcept
    {
        return layer_depth_as_byte_size(this->_depth) * this->_nchannels;
    }

    LOV_FORCE_INLINE bool is_loaded() const noexcept
    {
        return this->_data != nullptr;
    }

    LOV_FORCE_INLINE void allocate(const std::size_t nbytes) noexcept
    {
        if(this->_data != nullptr)
        {
            stdromano::mem_aligned_free(this->_data);
        }

        this->_data = stdromano::mem_aligned_alloc(nbytes, Layer::ALIGNMENT);
    }

    /* Pixel manipulation methods */

    void* get_pixel(std::int32_t x, std::int32_t y) const noexcept;

    void set_pixel(std::int32_t x, std::int32_t y, void* pixel) noexcept;

    void shuffle(const stdromano::StringD& mask) noexcept;

    void convert(const std::uint8_t new_depth) noexcept;

    bool compare(const Layer* other, const float tolerance = 0.001f) const noexcept;
};

using Layers = stdromano::HashMap<stdromano::StringD, Layer>;

class LOV_API Image
{
public:
/*
 * Equivalent to RGB, RGBA, in exr, or the only layer available if the file does not support layers
 */
static constexpr const char* MAIN_LAYER_NAME = "main";

private:
    friend class Layer;

    stdromano::StringD _path;

    Layers _layers;

    Imath::Box2i _data_window;
    Imath::Box2i _display_window;

    float _aspect_ratio;

    static bool read_image_metadata(const stdromano::StringD& path,
                                    Image& image) noexcept;

    static bool read_layer_pixels(const stdromano::StringD& path,
                                  const stdromano::StringD& layer_name,
                                  Layer& layer) noexcept;

public:
    Image() = default;

    Image(const stdromano::StringD& path) : _path(path)
    {
        Image::read_image_metadata(path, *this);
    }

    Image(const Imath::Box2i& data_window,
          const Imath::Box2i& display_window) : _data_window(data_window),
                                                _display_window(display_window),
                                                _aspect_ratio(1.0) {}

    const stdromano::StringD& get_path() const noexcept
    {
#if defined(LOV_PARANOID)
        LOV_ASSERT(!this->_path.empty(), "Image has not path");
#endif /* defined(LOV_PARANOID) */

        return this->_path;
    }

    /* Methods for layers */

    const Layers& get_layers() const noexcept { return this->_layers; }
    Layers& get_layers() noexcept { return this->_layers; }

    Layer* create_layer(const stdromano::StringD& name,
                        std::uint8_t depth,
                        std::uint8_t nchannels) noexcept;

    /* Returns the main layer (RGB, RGBA or the default) */
    Layer* main() noexcept;

    const Layer* main() const noexcept;

    /* Returns nullptr if the layer can't be found */
    /* Lazy loads the data of the layer if it exists */
    Layer* get_layer(const stdromano::StringD& name) noexcept;

    const Layer* get_layer(const stdromano::StringD& name) const noexcept;

    void remove_layer(const stdromano::StringD& name) noexcept;

    void rename_layer(const stdromano::StringD& name, const stdromano::StringD& new_name) noexcept;

    LOV_FORCE_INLINE bool has_layer(const stdromano::StringD& name) const noexcept
    {
        return this->_layers.contains(name);
    }

    /* Methods for data/display window */

    LOV_FORCE_INLINE const Imath::Box2i& data_window() const noexcept
    {
        return this->_data_window;
    }

    LOV_FORCE_INLINE Imath::Box2i& data_window() noexcept
    {
        return this->_data_window;
    }

    LOV_FORCE_INLINE const Imath::Box2i& display_window() const noexcept
    {
        return this->_display_window;
    }

    LOV_FORCE_INLINE Imath::Box2i& display_window() noexcept
    {
        return this->_display_window;
    }

    LOV_FORCE_INLINE std::int32_t get_data_width() const noexcept
    {
        return this->_data_window.max.x - this->_data_window.min.x + 1;
    }

    LOV_FORCE_INLINE std::int32_t get_display_width() const noexcept
    {
        return this->_display_window.max.x - this->_display_window.min.x + 1;
    }

    LOV_FORCE_INLINE std::int32_t get_data_height() const noexcept
    {
        return this->_data_window.max.y - this->_data_window.min.y + 1;
    }

    LOV_FORCE_INLINE std::int32_t get_display_height() const noexcept
    {
        return this->_display_window.max.y - this->_display_window.min.y + 1;
    }

    LOV_FORCE_INLINE const float& aspect_ratio() const noexcept
    {
        return this->_aspect_ratio;
    }

    LOV_FORCE_INLINE float& aspect_ratio() noexcept
    {
        return this->_aspect_ratio;
    }

    LOV_FORCE_INLINE bool is_valid() const noexcept { return this->_layers.size() > 0; }

    /* Methods for writing to disk */

    bool write(const stdromano::StringD& path) noexcept;

    /* Methods for manipulating */

    void crop(const Imath::Box2i& new_data_window) noexcept;

    void crop(std::int32_t min_x, std::int32_t min_y, std::int32_t max_x, std::int32_t max_y) noexcept;

    void resize(const Imath::Box2i& new_data_window,
                std::uint32_t mode = ResizeMode_BiLinear) noexcept;

    void resize(std::int32_t min_x,
                std::int32_t min_y,
                std::int32_t max_x,
                std::int32_t max_y,
                std::uint32_t mode = ResizeMode_BiLinear) noexcept;

    bool convert_colorspace(const stdromano::StringD& input_cs,
                    const stdromano::StringD& output_cs) noexcept;
};

LOV_FORCE_INLINE std::size_t Layer::npixels() const noexcept
{
    return static_cast<std::size_t>(this->_parent->get_display_width()) *
           static_cast<std::size_t>(this->_parent->get_display_height());
}


LOV_NAMESPACE_END

#endif /* !defined(__LOV_IMAGE) */
