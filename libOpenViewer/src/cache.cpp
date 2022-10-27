// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/cache.h"
#include "OpenViewerUtils/memory.h"

LOV_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
// Cache Item
////////////////////////////////////////////////////////////////////////////////

cache_item::cache_item(Media* media, void* ptr, const uint64_t stride, const uint64_t size, const uint32_t frame)
{
    this->m_media = media;
    this->m_data_ptr = ptr;
    this->m_stride = stride;
    this->m_size = size;
    this->m_frame = frame;

    this->m_media->set_cached_at_frame(this->m_frame, true);
}

cache_item::~cache_item()
{
    spdlog::debug("cache item dtor");
    this->m_media->set_cached_at_frame(this->m_frame, false);
}

////////////////////////////////////////////////////////////////////////////////
// Cache
////////////////////////////////////////////////////////////////////////////////

Cache::Cache(uint64_t size) 
{
    const uint64_t max_ram_capacity = lovu::get_total_system_memory();

    if(size >= max_ram_capacity)
    {
        size = max_ram_capacity / 2;

        spdlog::error("[CACHE] : Requested size is larger than available physical ram, setting cache size to {} bytes.",
                      size);
    }
    
    this->m_bytes_capacity = size;

    this->m_memory_arena = lovu::mem_alloc(size, 32);
    
    spdlog::debug("[CACHE] : Initialized cache ({} bytes)", size);
}

Cache::~Cache()
{
    if(this->m_memory_arena != nullptr)
    {
        lovu::mem_free(this->m_memory_arena);

        this->m_memory_arena = nullptr;
    }

    flush();

    spdlog::debug("[CACHE] : Released cache");
}

uint32_t Cache::add(const uint32_t hash, Media* item, const uint32_t frame) noexcept
{
    std::unique_lock<std::mutex> add_lock(this->m_mtx);

    const uint64_t item_img_size = item->get_size();
    const uint64_t item_img_byte_size = item->get_byte_size();

    // The image is bigger than the current cache capacity, resize it
    if(item_img_byte_size > this->m_bytes_capacity)
    {
        spdlog::debug("[CACHE] : Image size is bigger than cache capacity, resizing cache");

        this->resize(item_img_byte_size + 0xFF);
    }

    // The image we want to store fits in cache, lets give it a new address
    if((this->m_bytes_size + item_img_byte_size) <= this->m_bytes_capacity)
    {
        // The new cache address correspond to the current cache byte size
        const uint64_t cache_offset = this->m_bytes_size;

        // Calculate the new address
        void* new_img_address = static_cast<char*>(this->m_memory_arena) + cache_offset;

        // Create the new item
        this->m_items.emplace(this->m_current_index, cache_item(item, 
                                                                new_img_address, 
                                                                item_img_byte_size, 
                                                                item_img_size, 
                                                                frame));

        ++this->m_current_index;
        ++this->m_size;

        add_lock.unlock();
    }
}

void Cache::remove(const uint32_t hash) noexcept
{
    std::unique_lock<std::mutex> remove_lock(this->m_mtx);

    this->m_items[hash].m_media->set_cached_at_frame(this->m_items[hash].m_frame, false);
    this->m_bytes_size -= this->m_items[hash].m_stride;

    --this->m_current_index;
    --this->m_size;

    spdlog::debug("[CACHE] : Removed image {}", this->m_items[hash].m_media->get_path_view());

    remove_lock.unlock();
}

void Cache::flush() noexcept
{
    this->m_items.clear();
    this->m_size = 0;
    this->m_bytes_size = 0;
    this->m_current_index = 1;
    this->m_current_traversing_index = 0;

    spdlog::debug("[CACHE] : Cache flushed");
}

void Cache::resize(const size_t new_size) noexcept
{
    lovu::mem_free(this->m_memory_arena);

    this->m_memory_arena = lovu::mem_alloc(new_size, 32);
    this->m_bytes_capacity = new_size;

    this->flush();

    spdlog::debug("[CACHE] : Resized cache ({} bytes)", new_size);
}

void Cache::debug() const noexcept
{
    spdlog::debug("************************");
    spdlog::debug("[CACHE] : Debugging items that are in cache");   
    
    uint32_t i = 0;

    for(const auto& [key, value] : this->m_items)
    {
        spdlog::debug("------------------------------");
        spdlog::debug("| Index : {}", i);
        spdlog::debug("| Item : {} - {}", value.m_media->get_path_view(), value.m_frame);
    }
    spdlog::debug("------------------------------");
    
    spdlog::debug("************************");
}

uint32_t Cache::get_hash_from_index(const uint32_t index) const noexcept
{
}

LOV_NAMESPACE_END