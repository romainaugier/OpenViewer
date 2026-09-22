// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/media_cache.hpp"

#include "OpenViewer/log.hpp"

#include <new>

#include "stdromano/string.hpp"

LOV_NAMESPACE_BEGIN

// Formatted only when a message is actually written: log arguments are evaluated even
// when the level filters the message out, and allocate() runs once per cached frame
struct ByteSize
{
    std::size_t bytes;
};

LOV_NAMESPACE_END

template <>
struct fmt::formatter<lov::ByteSize>
{
    constexpr auto parse(fmt::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const lov::ByteSize& size, FormatContext& ctx) const -> decltype(ctx.out())
    {
        constexpr const char* const units[4] = {"Bytes", "Gb", "Mb", "Kb"};

        double value = static_cast<double>(size.bytes);
        std::size_t unit = 0;

        if(value > 1e9)
        {
            unit = 1;
            value /= 1e9;
        }
        else if(value > 1e6)
        {
            unit = 2;
            value /= 1e6;
        }
        else if(value > 1e3)
        {
            unit = 3;
            value /= 1e3;
        }

        return fmt::format_to(ctx.out(), "{:.02f} {}", value, units[unit]);
    }
};

LOV_NAMESPACE_BEGIN

MediaCache::MediaCache(std::size_t capacity) : _capacity(capacity),
                                               _size(0),
                                               _head(nullptr),
                                               _tail(nullptr),
                                               _write_ptr(nullptr)
{
    this->_buffer = static_cast<char*>(stdromano::mem_aligned_alloc(this->_capacity,
                                                                    ALIGNMENT));
    this->_write_ptr = _buffer;

    log_trace(LogCategory::MediaCache, "Initialized with {}", ByteSize{this->_capacity});
}

MediaCache::~MediaCache()
{
    std::lock_guard<std::recursive_mutex> lock(_mutex);

    while(this->_head != nullptr)
        this->free_oldest_block();

    if(this->_buffer != nullptr)
        stdromano::mem_free(this->_buffer);

    log_trace(LogCategory::MediaCache, "Destroyed a cache of {} bytes", this->_capacity);
}

std::size_t MediaCache::compute_total_size(std::size_t data_size) const noexcept
{
    return HEADER_SLOT + detail::round_up(data_size, ALIGNMENT);
}

void MediaCache::free_oldest_block() noexcept
{
    log_trace(LogCategory::MediaCache, "Freeing oldest block");

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

            log_trace(LogCategory::MediaCache, "Wrapping the write position around");

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
        log_trace(LogCategory::MediaCache, "Requested a 0 bytes block size, discarding");
        return nullptr;
    }

    const std::size_t total_sz = this->compute_total_size(data_sz);

    log_trace(LogCategory::MediaCache, "Requested a {} block", ByteSize{data_sz});

    if(total_sz > this->_capacity)
    {
        log_error(LogCategory::MediaCache,
                  "Requested block is too large ({} > {})",
                  total_sz,
                  this->_capacity);
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

    log_debug(LogCategory::MediaCache,
              "Allocated a new block ({} | {})",
              fmt::ptr(data_ptr),
              ByteSize{data_sz});

    log_trace(LogCategory::MediaCache,
              "Occupancy: {}/{}",
              ByteSize{this->_size},
              ByteSize{this->_capacity});

    return data_ptr;
}

void MediaCache::clear() noexcept
{
    std::lock_guard<std::recursive_mutex> lock(_mutex);

    while(this->_head != nullptr)
        this->free_oldest_block();

    this->_write_ptr = this->_buffer;

    log_trace(LogCategory::MediaCache, "Cleared");
}

LOV_NAMESPACE_END
