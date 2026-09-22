// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/media_cache.hpp"

#include "OpenViewer/log.hpp"

#include <new>

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

    log_trace("Initialized with {}", format_byte_size(this->_capacity));
}

MediaCache::~MediaCache()
{
    std::lock_guard<std::recursive_mutex> lock(_mutex);

    while(this->_head != nullptr)
        this->free_oldest_block();

    if(this->_buffer != nullptr)
        stdromano::mem_free(this->_buffer);

    log_trace("Destroyed a cache of {} bytes", this->_capacity);
}

std::size_t MediaCache::compute_total_size(std::size_t data_size) const noexcept
{
    return HEADER_SLOT + detail::round_up(data_size, ALIGNMENT);
}

void MediaCache::free_oldest_block() noexcept
{
    log_trace("Freeing oldest block");

    BlockHeader* header = this->_head;

    if(header == nullptr)
        return;

    if(header->dtor != nullptr)
        header->dtor();

    this->_size -= header->total_sz;

    this->_head = header->next;

    if(this->_head == nullptr)
        this->_tail = nullptr;

    header->~BlockHeader();
}

void MediaCache::make_space(std::size_t total_sz) noexcept
{
    char* const buffer_end = this->_buffer + this->_capacity;

    while(true)
    {
        if(this->_head == nullptr)
        {
            this->_write_ptr = this->_buffer;
            return;
        }

        char* const head = reinterpret_cast<char*>(this->_head);

        if(head < this->_write_ptr)
        {
            if(static_cast<std::size_t>(buffer_end - this->_write_ptr) >= total_sz)
                return;

            log_trace("Wrapping the write position around");

            this->_write_ptr = this->_buffer;
            continue;
        }

        if(static_cast<std::size_t>(head - this->_write_ptr) >= total_sz)
            return;

        this->free_oldest_block();
    }
}

void* MediaCache::allocate(std::size_t data_sz, std::function<void()> dtor) noexcept
{
    if(this->_buffer == nullptr)
        return nullptr;

    std::lock_guard<std::recursive_mutex> lock(this->_mutex);

    if(data_sz == 0)
    {
        log_trace("Requested a 0 bytes block size, discarding");
        return nullptr;
    }

    const std::size_t total_sz = this->compute_total_size(data_sz);

    log_trace("Requested a {} block", format_byte_size(data_sz));

    if(total_sz > this->_capacity)
    {
        log_error("Requested block is too large ({} > {})", total_sz, this->_capacity);
        return nullptr;
    }

    this->make_space(total_sz);

    BlockHeader* header = ::new(this->_write_ptr) BlockHeader{data_sz,
                                                              total_sz,
                                                              std::move(dtor),
                                                              nullptr,
                                                              HEADER_SLOT - sizeof(BlockHeader)};

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

    log_debug("Allocated a new block ({} | {})", fmt::ptr(data_ptr), format_byte_size(data_sz));
    log_trace("Occupancy: {}/{}", format_byte_size(this->_size), format_byte_size(this->_capacity));

    return data_ptr;
}

void MediaCache::clear() noexcept
{
    std::lock_guard<std::recursive_mutex> lock(_mutex);

    while(this->_head != nullptr)
        this->free_oldest_block();

    this->_write_ptr = this->_buffer;

    log_trace("Cleared");
}

LOV_NAMESPACE_END
