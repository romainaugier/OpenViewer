// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__LOV_MEDIA_CACHE)
#define __LOV_MEDIA_CACHE

#include "OpenViewer/common.hpp"
#include "OpenViewer/log.hpp"

#include "stdromano/memory.hpp"

#include <functional>
#include <mutex>

LOV_NAMESPACE_BEGIN

DETAIL_NAMESPACE_BEGIN

LOV_FORCE_INLINE constexpr std::size_t round_up(std::size_t value, std::size_t alignment) noexcept
{
    return (value + alignment - 1) / alignment * alignment;
}

DETAIL_NAMESPACE_END

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

    static constexpr std::size_t HEADER_SLOT = detail::round_up(sizeof(BlockHeader), ALIGNMENT);

    static_assert(ALIGNMENT % alignof(BlockHeader) == 0,
                  "headers placed on ALIGNMENT boundaries must be correctly aligned");

    char* _buffer;
    std::size_t _capacity;
    std::size_t _size;
    BlockHeader* _head;
    BlockHeader* _tail;
    char* _write_ptr;
    mutable std::recursive_mutex _mutex;

    std::size_t compute_total_size(std::size_t data_size) const noexcept;

    LOV_FORCE_INLINE void* get_data_ptr(BlockHeader* header) const noexcept
    {
        return reinterpret_cast<char*>(header) + HEADER_SLOT;
    }

    LOV_FORCE_INLINE BlockHeader* get_header_from_data(void* data) const noexcept
    {
        return reinterpret_cast<BlockHeader*>(reinterpret_cast<char*>(data) - HEADER_SLOT);
    }

    void free_oldest_block() noexcept;

    void make_space(std::size_t total_sz) noexcept;

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
        return this->_size;
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
