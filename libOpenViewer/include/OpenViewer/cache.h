// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#include "OpenViewer/media.h"

#include "tsl/robin_map.h"

#include <mutex>

LOV_NAMESPACE_BEGIN

// Image cache that works like a circular buffer
// We store cache items that have a pointer to an image (to gather
// the different informations we need about the image), and its stride
// (the size of the image in bytes to ease the allocation of memory for
// the next item)
// Each image can have a different size, we keep track of the allocated space for this image using
// the stride of each image. If an image that needs to be cached is smaller
// than the one already cached, we keep the real stride in the m_Stride member (in bytes).
// If the image is larger, we will release images until enough space if free, 
// and allocate the necessary memory for this "big" image.

// Holds information about the image it is caching. 
// Used internally by the image cache
struct LOV_API cache_item
{
    Media* m_media;
    void* m_data_ptr;
    
    uint64_t m_stride;
    uint64_t m_size; 

    uint32_t m_frame;

    cache_item();
    cache_item(Media* media, void* ptr, const uint64_t stride, const uint64_t size, const uint32_t frame);
    ~cache_item();
};

class LOV_API Cache
{
public:
    // Constructs the cache from the settings
    Cache();

    // Constructs the cache. Size needs to be in bytes
    Cache(uint64_t size);

    // Destructs the cache, clear everything and free the memory
    ~Cache();

    // Adds an image to the cache, and it will remove one at the beginning (or more) if the cache is full
    // Returns the memory address where the image data should be loaded
    void* add(Media* item, const uint32_t frame) noexcept;

    // Removes an image from the cache
    void remove(const uint32_t index) noexcept;

    // Clears all the items, but keeps the allocated memory available
    void flush() noexcept;

    // Resizes the cache. Size needs to be in bytes
    void resize(size_t new_size) noexcept;
    
    // Print the cache to the console for debugging purpose
    void debug() const noexcept;

    // Returns a ptr to an item cache given a hash (return nullptr if the item is not in cache)
    cache_item* get_cache_item(const uint32_t hash) const noexcept;

private:
    // Store the adresses to images (and some other informations)
    // The key is the cache index
    tsl::robin_map<uint32_t, cache_item> m_items;

    // Store the pointers to associated image cache item
    // The key is the path hash
    tsl::robin_map<uint32_t, cache_item*> m_hash_to_items;

    std::mutex m_mtx;

    void* m_memory_arena = nullptr;
    
    uint64_t m_max_capacity = 0;

    uint64_t m_bytes_capacity = 0;
    uint64_t m_bytes_size = 0;

    // Buffer size, capacity and current index of allocation
    uint32_t m_current_index = 1;
    uint32_t m_current_traversing_index = 0;
    uint32_t m_size = 0;
    uint32_t m_capacity = 0;
};

LOV_NAMESPACE_END