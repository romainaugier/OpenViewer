// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/cache.h"
#include "OpenViewer/settings.h"
#include "OpenViewerUtils/memory.h"

LOV_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
// Cache Item
////////////////////////////////////////////////////////////////////////////////

cache_item::cache_item()
{
    this->m_media = nullptr;
    this->m_data_ptr = nullptr;
    this->m_stride = 0;
    this->m_size = 0;
    this->m_frame = 0;
}

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
    // this->m_media->set_cached_at_frame(this->m_frame, false);
}

////////////////////////////////////////////////////////////////////////////////
// Cache
////////////////////////////////////////////////////////////////////////////////

Cache::Cache()
{
    const uint64_t max_ram_capacity = lovu::get_total_system_memory();

    spdlog::debug("[CACHE] : Detected max ram capacity : {} Gb", lovu::to_gb(max_ram_capacity));

    const uint8_t cache_mode = lov::settings.get<uint8_t>("cache_mode");

    if(cache_mode == 1)
    {
        this->m_max_capacity = LOVU_CAST(uint64_t, lovu::from_mb(lov::settings.get<uint32_t>("cache_max_size")));
    }
    else if(cache_mode == 2)
    {
        this->m_max_capacity = LOVU_CAST(uint64_t, lovu::from_mb(lov::settings.get<uint32_t>("cache_max_ram_usage"))) / 100.0f * max_ram_capacity;
    }
}

Cache::Cache(uint64_t size) 
{
    const uint64_t max_ram_capacity = lovu::get_total_system_memory();

    spdlog::debug("[CACHE] : Detected max ram capacity : {} Gb", lovu::to_gb(max_ram_capacity));

    const uint8_t cache_mode = lov::settings.get<uint8_t>("cache_mode");

    if(cache_mode == 1)
    {
        this->m_max_capacity = LOVU_CAST(uint64_t, lovu::from_mb(lov::settings.get<uint32_t>("cache_max_size")));
    }
    else if(cache_mode == 2)
    {
        this->m_max_capacity = LOVU_CAST(uint64_t, max_ram_capacity * lov::settings.get<uint32_t>("cache_max_ram_usage"));
    }

    if(size >= max_ram_capacity || size >= this->m_max_capacity)
    {
        size = max_ram_capacity / 2;

        spdlog::error("[CACHE] : Requested size is larger than available physical ram, setting cache size to {} bytes.",
                      size);
    }
    
    this->m_bytes_capacity = size;
    this->m_max_capacity = size;

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

void* Cache::add(Media* media, const uint32_t frame) noexcept
{
    std::unique_lock<std::mutex> add_lock(this->m_mtx);

    const uint64_t item_img_size = media->get_size();
    const uint64_t item_img_byte_size = media->get_byte_size();

    // The image is bigger than the current cache capacity, resize it
    if(item_img_byte_size > this->m_bytes_capacity)
    {
        spdlog::debug("[CACHE] : Image size is bigger than cache capacity, resizing cache");

        this->resize(item_img_byte_size + 0xff);
    }

    if((this->m_bytes_size + item_img_byte_size) <= this->m_bytes_capacity)
    {
        // The image we want to store fits in cache, lets give it a new address
        
        // The new cache address correspond to the current cache byte size
        const uint64_t cache_offset = this->m_bytes_size;

        // Calculate the new address
        void* new_img_address = static_cast<char*>(this->m_memory_arena) + cache_offset;

        // Create the new cache item
        this->m_items.emplace(this->m_current_index, cache_item(media, 
                                                                new_img_address, 
                                                                item_img_byte_size, 
                                                                item_img_size, 
                                                                frame));

        const uint32_t hash = media->get_hash_at_frame(frame);
        this->m_hash_to_items[hash] = this->m_current_index;

        spdlog::debug("[CACHE] : Loaded image \"{} - {}\" at index {}", 
                                  media->get_path_view(),
                                  frame,
                                  this->m_current_index);

        this->m_bytes_size += item_img_byte_size;

        ++this->m_current_index;
        ++this->m_size;

        add_lock.unlock();

        return new_img_address;
    }
    else
    {
        // The image we want to store does not fit in the cache. We will release images until
        // we have enough space to store it
        const uint32_t current_index = this->m_current_traversing_index == 0 ? 0 : this->m_current_traversing_index % this->m_size;
        const uint32_t old_size = this->m_size;

        uint32_t traversing_index = current_index + 1;
        uint64_t cleaned_byte_size = 0;
        uint32_t clean_index = 1;
        void* clean_address = nullptr;

        while(true)
        {
            traversing_index = traversing_index % (old_size + 1) == 0 ? 1 : traversing_index;
            cache_item tmp_cache_item = this->m_items[traversing_index];  

            const bool image_fits_in_cache = item_img_byte_size <= (this->m_bytes_capacity - (this->m_bytes_size - tmp_cache_item.m_stride));

            if(item_img_byte_size <= tmp_cache_item.m_stride || image_fits_in_cache)
            {
                // We found an image that was the same size (or larger), or we have enough space in the cache to load the frame at the
                // current index. We remove it from the cache and use its memory to store our new cache

                // Notify that this image is not in cache anymore and update the cache infos
                tmp_cache_item.m_media->set_cached_at_frame(tmp_cache_item.m_frame, false);

                this->m_bytes_size -= tmp_cache_item.m_stride;

                void* address = tmp_cache_item.m_data_ptr;

                if(image_fits_in_cache)
                {
                    // Special case where we emptied all the cache but we are not at index 1, so we kind of flush the cache
                    // and add a new item at the index 1
                    if(this->m_items.find(1) == this->m_items.end())
                    {
                        this->m_items.erase(traversing_index);
                        address = this->m_memory_arena;
                        traversing_index = 1;
                    }
                }

                this->m_items[traversing_index] = cache_item(media, 
                                                             address, 
                                                             item_img_byte_size, 
                                                             item_img_size, 
                                                             frame);

                const uint32_t hash = media->get_hash_at_frame(frame);
                this->m_hash_to_items[hash] = traversing_index;

                // Update cache infos
                this->m_bytes_size += item_img_byte_size;

                this->m_current_traversing_index = traversing_index;
                this->m_current_index = traversing_index + 1;

                spdlog::debug("[CACHE] : Removed image \"{} - {}\" at index {} and loaded a new one \"{} - {}\"", 
                               tmp_cache_item.m_media->get_path_view(),
                               tmp_cache_item.m_frame,
                               traversing_index,
                               media->get_path_view(),
                               frame);

                tmp_cache_item.m_media->set_cached_at_frame(tmp_cache_item.m_frame, false);

                add_lock.unlock();

                return address;
            }
            else
            {
                // Begin the cleanup, get the address of the first released image and item for the index
                if(cleaned_byte_size == 0)
                {
                    clean_address = tmp_cache_item.m_data_ptr;
                    clean_index = traversing_index;
                }

                if(item_img_byte_size <= cleaned_byte_size || item_img_byte_size <= this->m_bytes_size)
                {
                    this->m_items[clean_index] = cache_item(media, clean_address, item_img_byte_size, item_img_size, frame);
                    
                    const uint32_t hash = media->get_hash_at_frame(frame);
                    this->m_hash_to_items[hash] = clean_index;

                    this->m_current_traversing_index = clean_index;
                    this->m_current_index = traversing_index + 1;

                    spdlog::debug("[CACHE] : Loaded image \"{} - {}\" at index {}", 
                                  media->get_path_view(),
                                  frame,
                                  traversing_index);

                    return clean_address;
                }
                else
                {
                    this->m_size -= 1;

                    this->m_bytes_size -= tmp_cache_item.m_stride;
                    cleaned_byte_size += tmp_cache_item.m_stride;

                    spdlog::debug("[CACHE] : Removed image \"{} - {}\" at index {}",
                                  tmp_cache_item.m_media->get_path_view(),
                                  tmp_cache_item.m_frame,
                                  traversing_index);

                    this->remove(traversing_index);
                }
            }

            ++traversing_index;
        }
    }
}

void Cache::remove(const uint32_t index) noexcept
{
    std::unique_lock<std::mutex> remove_lock(this->m_mtx);

    const uint32_t frame = this->m_items[index].m_frame;
    const uint32_t hash = this->m_items[index].m_media->get_hash_at_frame(frame);

    this->m_items[index].m_media->set_cached_at_frame(frame, false);
    this->m_bytes_size -= this->m_items[index].m_stride;

    this->m_items.erase(index);
    this->m_hash_to_items.erase(hash);

    --this->m_current_index;
    --this->m_size;

    spdlog::debug("[CACHE] : Removed image {}", this->m_items[index].m_media->get_path_view());

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

void Cache::resize(size_t new_size) noexcept
{
    if(new_size > this->m_max_capacity) 
    {
        new_size = this->m_max_capacity;
    
        spdlog::warn("[CACHE] : Requested new size larger than maximum capacity, setting new size to maximum capacity");
    }

    if(this->m_memory_arena != nullptr) lovu::mem_free(this->m_memory_arena);

    this->m_memory_arena = lovu::mem_alloc(new_size, 32);
    this->m_bytes_capacity = new_size;

    spdlog::debug("[CACHE] : New memory address : {}", fmt::ptr(this->m_memory_arena));

    this->flush();

    spdlog::debug("[CACHE] : Resized cache ({} mb)", lovu::to_mb(new_size));
}

void Cache::debug() const noexcept
{
    spdlog::debug("************************");
    spdlog::debug("[CACHE] : Debugging items that are in cache");   
    spdlog::debug("Size (gb) : {}", lovu::to_gb(this->m_bytes_size));
    spdlog::debug("Capacity (gb) : {}", lovu::to_gb(this->m_bytes_capacity));
    
    for(uint32_t i = 1; i < this->m_size + 1; i++)
    {
        const cache_item item = this->m_items.at(i);

        spdlog::debug("------------------------------");
        spdlog::debug("| Index : {}", i);
        spdlog::debug("| Item : {} - {}", item.m_media->get_path_view(), item.m_frame);
    }
    spdlog::debug("------------------------------");
    
    spdlog::debug("************************");
}

cache_item* Cache::get_cache_item(const uint32_t hash) const noexcept
{
    auto it = this->m_hash_to_items.find(hash);

    if(it != this->m_hash_to_items.end())
    {
        const uint32_t item_index = it.value();
        auto item_it = this->m_items.find(item_index);

        if(item_it != this->m_items.end())
        {
            return const_cast<cache_item*>(&item_it.value());
        }
    }

    return nullptr;
}

LOV_NAMESPACE_END