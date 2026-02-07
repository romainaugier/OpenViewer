// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/media_cache.hpp"

#include "stdromano/string.hpp"

LOV_NAMESPACE_BEGIN

constexpr const char* const units[4] = { "Bytes", "Gb", "Mb", "Kb" };

stdromano::StringD format_byte_size(float size) noexcept
{
    std::size_t unit = 0;

    if(size > 1e9)
    {
        unit = 1;
        size = size / 1e9;
    }
    else if(size > 1e6)
    {
        unit = 2;
        size = size / 1e6;
    }
    else if(size > 1e3)
    {
        unit = 3;
        size = size / 1e3;
    }

    return stdromano::StringD("{:.02f} {}", size, units[unit]);
}

MediaCache::MediaCache(std::size_t capacity) : _capacity(capacity),
                                               _size(0),
                                               _head(nullptr),
                                               _tail(nullptr),
                                               _write_ptr(nullptr)
{
    this->_buffer = static_cast<char*>(stdromano::mem_aligned_alloc(this->_capacity,
                                                                    ALIGNMENT));
    this->_write_ptr = _buffer;

    this->_logger = spdlog::get("media_cache");

    if(this->_logger == nullptr)
    {
        spdlog::error("Cannot get media_cache logger");
    }

    this->_logger->trace("Initialized with {}", format_byte_size(this->_capacity));
}

MediaCache::~MediaCache()
{
    std::lock_guard<std::recursive_mutex> lock(_mutex);

    while(this->_head != nullptr)
        this->free_oldest_block();

    if(this->_buffer != nullptr)
    {
        stdromano::mem_free(this->_buffer);
    }

    this->_logger->trace("Destroyed", this->_capacity);
}

std::size_t MediaCache::compute_total_size(std::size_t data_size) const noexcept
{
    std::uintptr_t data_start = reinterpret_cast<std::uintptr_t>(this->_write_ptr) + sizeof(BlockHeader);
    std::size_t data_padding = (ALIGNMENT - (data_start % ALIGNMENT)) % ALIGNMENT;

    return sizeof(BlockHeader) + data_padding + data_size;
}

void MediaCache::free_oldest_block() noexcept
{
    this->_logger->trace("Freeing oldest block");

    if(this->_head == nullptr)
        return;

    if(this->_head->dtor != nullptr)
        this->_head->dtor();

    this->_size -= this->_head->total_sz;

    this->_head = this->_head->next;

    if(this->_head == nullptr)
        this->_tail = nullptr;
}

void MediaCache::make_space(std::size_t required_size) noexcept
{
    std::size_t remaining = this->get_free_bytes();

    if(required_size > remaining)
    {
        this->_logger->trace("Making space (required: {})", format_byte_size(required_size));

        this->_write_ptr = nullptr;
    }

    while(this->_head != nullptr && required_size > remaining)
    {
        if(this->_write_ptr == nullptr)
            this->_write_ptr = reinterpret_cast<char*>(this->_head);

        this->free_oldest_block();

        remaining = this->get_free_bytes();
    }
}

void* MediaCache::allocate(std::size_t data_sz, std::function<void()> dtor) noexcept
{
    if(this->_buffer == nullptr)
        return nullptr;

    std::lock_guard<std::recursive_mutex> lock(this->_mutex);

    if(data_sz == 0)
    {
        this->_logger->trace("Requested a 0 bytes block size, discading");
        return nullptr;
    }

    std::uintptr_t data_start = reinterpret_cast<std::uintptr_t>(this->_write_ptr) + sizeof(BlockHeader);
    std::size_t data_padding = (ALIGNMENT - (data_start % ALIGNMENT)) % ALIGNMENT;
    std::size_t total_sz = sizeof(BlockHeader) + data_padding + data_sz;

    this->_logger->trace("Requested a {} block", format_byte_size(data_sz));

    if(total_sz > this->_capacity)
    {
        this->_logger->error("Requested block is too large ({} > {})",
                             total_sz,
                             this->_capacity);
        return nullptr;
    }

    this->make_space(total_sz);

    BlockHeader* header = reinterpret_cast<BlockHeader*>(this->_write_ptr);
    header->data_sz = data_sz;
    header->total_sz = total_sz;
    header->dtor = std::move(dtor);
    header->next = nullptr;
    header->padding = data_padding;

    void* data_ptr = this->get_data_ptr(header);

    if(this->_head == nullptr)
    {
        this->_head = this->_tail = header;
    }
    else
    {
        this->_tail->next = header;
        this->_tail = header;
    }

    this->_write_ptr += total_sz;
    this->_size += total_sz;

    this->_logger->debug("Allocated a new block at {}", fmt::ptr(data_ptr));
    this->_logger->trace("Occupancy: {}/{}",
                         format_byte_size(this->_size),
                         format_byte_size(this->_capacity));

    return data_ptr;
}

void MediaCache::clear() noexcept
{
    std::lock_guard<std::recursive_mutex> lock(_mutex);

    while(this->_head != nullptr)
        this->free_oldest_block();

    this->_write_ptr = this->_buffer;

    this->_logger->trace("Cleared");
}

LOV_NAMESPACE_END
