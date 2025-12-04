// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/image.hpp"

#include "stdromano/logger.hpp"

LOV_NAMESPACE_BEGIN

/* Helpers */

std::size_t layer_depth_as_byte_size(std::uint8_t layer_depth) noexcept
{
    switch(layer_depth)
    {
        case LayerDepth_NONE:
            return 0;
        case LayerDepth_U8:
        case LayerDepth_F8:
            return 1;
        case LayerDepth_U16:
        case LayerDepth_F16:
            return 2;
        case LayerDepth_F32:
        case LayerDepth_U32:
            return 4;
        default:
            return 0;
    }
}

/* Layer */

Layer::~Layer() noexcept
{
    if(this->_data != nullptr)
    {
        stdromano::mem_aligned_free(this->_data);
        this->_data = nullptr;
    }
}

Layer::Layer(const Layer& other) : _parent(other._parent),
                                   _depth(other._depth),
                                   _nchannels(other._nchannels)
{
    this->_data = stdromano::mem_aligned_alloc(this->nbytes(), ALIGNMENT);
    std::memcpy(this->_data, other._data, this->nbytes());
}

Layer& Layer::operator=(const Layer& other) noexcept
{
    if(this != &other)
    {
        if(this->_data != nullptr)
        {
            stdromano::mem_aligned_free(this->_data);
        }

        this->_parent = other._parent;
        this->_depth = other._depth;
        this->_nchannels = other._nchannels;

        if(other._data != nullptr)
        {
            this->_data = stdromano::mem_aligned_alloc(this->nbytes(), ALIGNMENT);
            std::memcpy(this->_data, other._data, this->nbytes());
        }
    }

    return *this;
}

Layer::Layer(Layer&& other) noexcept : _parent(other._parent),
                                       _depth(other._depth),
                                       _nchannels(other._nchannels),
                                       _data(other._data)
{
    other._parent = nullptr;
    other._depth = 0;
    other._nchannels = 0;
    other._data = nullptr;
}

Layer& Layer::operator=(Layer&& other) noexcept
{
    if(this != &other)
    {
        if(this->_data != nullptr)
        {
            stdromano::mem_aligned_free(this->_data);
        }

        this->_parent = other._parent;
        this->_depth = other._depth;
        this->_nchannels = other._nchannels;
        this->_data = other._data;

        other._parent = nullptr;
        other._depth = 0;
        other._nchannels = 0;
        other._data = nullptr;
    }

    return *this;
}

void* Layer::get_pixel(const std::int32_t x, const std::int32_t y) const noexcept
{
    if(this->_data == nullptr)
    {
        return nullptr;
    }

    LOV_ASSERT(x >= this->_parent->data_window().min.x &&
                  x < this->_parent->data_window().max.x &&
                  y >= this->_parent->data_window().min.y &&
                  y < this->_parent->data_window().max.y,
                  "Out-of-bounds pixel access");

    const std::size_t offset = x + y * this->_parent->get_data_width();

    return static_cast<void*>(std::addressof(static_cast<char*>(this->_data)[offset]));
}

void Layer::set_pixel(std::int32_t x, std::int32_t y, void* pixel) noexcept
{
    if(this->_data == nullptr)
    {
        return;
    }

    LOV_ASSERT(x >= this->_parent->data_window().min.x &&
                  x < this->_parent->data_window().max.x &&
                  y >= this->_parent->data_window().min.y &&
                  y < this->_parent->data_window().max.y,
                  "Out-of-bounds pixel access");

    const std::size_t offset = x + y * this->_parent->get_data_width();

    std::memcpy(std::addressof(static_cast<char*>(this->_data)[offset]),
                pixel,
                this->_nchannels * layer_depth_as_byte_size(this->_depth));
}

/* Image */

Layer* Image::create_layer(const stdromano::StringD& name,
                           std::uint8_t depth,
                           std::uint8_t nchannels) noexcept
{
    auto [it, _] = this->_layers.emplace(std::make_pair(name, Layer(this, depth, nchannels)));

    return std::addressof(it->second);
}

Layer* Image::main() noexcept
{
    return this->get_layer(Image::MAIN_LAYER_NAME);
}

const Layer* Image::main() const noexcept
{
    return this->get_layer(Image::MAIN_LAYER_NAME);
}

Layer* Image::get_layer(const stdromano::StringD& name) noexcept
{
    auto it = this->_layers.find(name);

    if(it == this->_layers.end())
    {
        return nullptr;
    }

    Layer& layer = it->second;

    if(!layer.is_loaded())
    {
        layer.allocate(layer.nbytes());

        if(!Image::read_layer_pixels(layer.parent()->get_path(), name, layer))
        {
            stdromano::log_error("Error during pixel read of layer {} of image {}",
                                 name,
                                 this->_path);
        }
    }

    return std::addressof(layer);
}

const Layer* Image::get_layer(const stdromano::StringD& name) const noexcept
{
    auto it = this->_layers.find(name);

    if(it == this->_layers.end())
    {
        return nullptr;
    }

    const Layer& layer = it->second;

    return std::addressof(layer);
}

void Image::remove_layer(const stdromano::StringD& name) noexcept
{
    auto it = this->_layers.find(name);

    if(it != this->_layers.end())
    {
        this->_layers.erase(it);
    }
}

void Image::rename_layer(const stdromano::StringD& name, const stdromano::StringD& new_name) noexcept
{
    auto it = this->_layers.find(name);

    if(it != this->_layers.end())
    {
        Layer layer = std::move(it->second);
        this->_layers.erase(it);
        this->_layers.insert(std::make_pair(new_name, std::move(layer)));
    }
}

LOV_NAMESPACE_END
