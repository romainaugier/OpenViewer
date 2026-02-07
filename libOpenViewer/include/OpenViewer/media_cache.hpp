// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__LOV_MEDIA_CACHE)
#define __LOV_MEDIA_CACHE

#include "OpenViewer/common.hpp"

#include "stdromano/memory.hpp"

#include <spdlog/spdlog.h>

#include <mutex>
#include <functional>

LOV_NAMESPACE_BEGIN

class LOV_API MediaCache
{
private:
    struct BlockHeader
    {
        std::size_t data_sz;
        std::size_t total_sz;
        std::function<void()> dtor;
        BlockHeader* next;
        std::size_t padding;
    };

    static constexpr std::size_t ALIGNMENT = 32;

    char* _buffer;
    std::size_t _capacity;
    std::size_t _size;
    BlockHeader* _head;
    BlockHeader* _tail;
    char* _write_ptr;
    mutable std::recursive_mutex _mutex;

    std::shared_ptr<spdlog::logger> _logger;

    std::size_t compute_total_size(std::size_t data_size) const noexcept;

    LOV_FORCE_INLINE void* get_data_ptr(BlockHeader* header) const noexcept
    {
        return reinterpret_cast<char*>(header) + sizeof(BlockHeader) + header->padding;
    }

    LOV_FORCE_INLINE BlockHeader* get_header_from_data(void* data) const noexcept
    {
        return reinterpret_cast<BlockHeader*>(reinterpret_cast<char*>(data) - sizeof(BlockHeader));
    }

    void free_oldest_block() noexcept;

    void make_space(std::size_t required_size) noexcept;

public:
    MediaCache(std::size_t capacity);

    ~MediaCache();

    MediaCache(const MediaCache&) = delete;
    MediaCache& operator=(const MediaCache&) = delete;

    void* allocate(std::size_t data_sz,
                   std::function<void()> dtor = []() -> void { return; }) noexcept;

    LOV_FORCE_INLINE std::size_t get_used_bytes() const noexcept
    {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return this->_write_ptr - this->_buffer;
    }

    LOV_FORCE_INLINE std::size_t get_capacity() const noexcept
    {
        return this->_capacity;
    }

    LOV_FORCE_INLINE std::size_t get_free_bytes() const noexcept
    {
        std::lock_guard<std::recursive_mutex> lock(this->_mutex);
        return this->_capacity - this->_size;
    }

    void clear() noexcept;
};

LOV_NAMESPACE_END

#endif // #if define(__LOV_MEDIA_CACHE)
