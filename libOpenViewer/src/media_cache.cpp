// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/media_cache.hpp"

LOV_NAMESPACE_BEGIN

std::size_t MediaCache::compute_total_size(std::size_t data_size) const noexcept
{
    std::uintptr_t data_start = reinterpret_cast<std::uintptr_t>(this->_write_ptr) + sizeof(BlockHeader);
    std::size_t data_padding = (ALIGNMENT - (data_start % ALIGNMENT)) % ALIGNMENT;

    return sizeof(BlockHeader) + data_padding + data_size;
}

void MediaCache::free_oldest_block() noexcept
{
    this->_logger->trace("Freeing older block");

    if(this->_head == nullptr)
        return;

    if(this->_head->dtor != nullptr)
        this->_head->dtor();

    this->_head = this->_head->next;

    if(this->_head == nullptr)
        this->_tail = nullptr;
}

void MediaCache::make_space(std::size_t required_size) noexcept
{
    this->_logger->trace("Making space (required: {} bytes)", required_size);

    std::size_t remaining = this->_capacity - (this->_write_ptr - this->_buffer);

    if(required_size > remaining)
    {
        this->_write_ptr = this->_buffer;
        this->_head = this->_tail = nullptr;
        remaining = this->_capacity;
    }

    while(this->_head != nullptr && required_size > remaining)
    {
        BlockHeader* old_head = this->_head;

        this->free_oldest_block();

        char* old_block_start = reinterpret_cast<char*>(old_head);

        if (old_block_start >= this->_write_ptr)
            remaining = this->_capacity - (this->_write_ptr - this->_buffer);
    }
}

void* MediaCache::allocate(std::size_t data_sz, std::function<void()> dtor) noexcept
{
    if(this->_buffer == nullptr)
        return nullptr;

    std::lock_guard<std::mutex> lock(_mutex);

    if(data_sz == 0)
    {
        this->_logger->trace("Requested a 0 bytes block size, discading");
        return nullptr;
    }

    std::uintptr_t data_start = reinterpret_cast<std::uintptr_t>(this->_write_ptr) + sizeof(BlockHeader);
    std::size_t data_padding = (ALIGNMENT - (data_start % ALIGNMENT)) % ALIGNMENT;
    std::size_t total_sz = sizeof(BlockHeader) + data_padding + data_sz;

    this->_logger->trace("Requested a {} bytes block size", data_sz);

    if(total_sz > this->_capacity)
    {
        this->_logger->error("Requested block is too large ({} > {})",
                             total_sz,
                             this->_capacity);
        return nullptr;
    }

    this->make_space(total_sz);

    BlockHeader* header = reinterpret_cast<BlockHeader*>(this->_write_ptr);
    header->data_size = data_sz;
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

    this->_logger->debug("Allocated a new block at {}", fmt::ptr(data_ptr));

    return data_ptr;
}

void MediaCache::clear() noexcept
{
    std::lock_guard<std::mutex> lock(_mutex);

    while(this->_head != nullptr)
        this->free_oldest_block();

    this->_write_ptr = this->_buffer;

    this->_logger->trace("Cleared");
}

LOV_NAMESPACE_END
