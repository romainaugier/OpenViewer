// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include <unordered_map>
#include <mutex>

#include "image.h"
#include "utils/memory/alloc.h"
#include "utils/logger.h"
#include "tsl/robin_map.h"

namespace Core
{

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

    struct ImageCacheItem
    {
        Image* m_Image;
        void* m_DataPtr;
        
        uint64_t m_Stride;
        uint64_t m_Size; 

        uint32_t m_MediaId;

        ImageCacheItem() {}

        ImageCacheItem(Image* image, void* ptr, const uint64_t stride, const uint64_t size, const uint32_t mediaId) : 
            m_Image(image),
            m_DataPtr(ptr),
            m_Stride(stride),
            m_Size(size),
            m_MediaId(mediaId) {}
    };

    using ImageCacheMap = tsl::robin_map<uint32_t, ImageCacheItem>;

    struct ImageCache
    {
        // Store the adresses to images (and some other informations)
        ImageCacheMap m_Items;

        std::mutex m_Mtx;

        void* m_MemoryArena = nullptr;
        
        Logger* m_Logger = nullptr;

        uint64_t m_BytesCapacity = 0;
        uint64_t m_BytesSize = 0;

        // Buffer size, capacity and current index of allocation
        uint32_t m_CurrentIndex = 1; // The index starts at 1, because we store the index in an unsigned 32 bits integer (and not cached is indicated by an index of 0)
        uint32_t m_CurrentTraversingIndex = 0;
        uint32_t m_Size = 0;
        uint32_t m_Capacity = 0;

        bool m_HasBeenInitialized = false;

        // Initializes the cache, the size needs to be expressed in MB
        void Initialize(const size_t size, Logger* logger, const bool sizeInMB = false) noexcept;

        // Adds an image to the cache, and it will remove one at the beginning (or more) is the cache is full
        uint32_t Add(Image* item, const uint32_t mediaId) noexcept;

        // Remove an image from the cache
        void Remove(const uint32_t index) noexcept;

        // Clear all the items, but keeps the allocated memory available
        void Flush() noexcept;

        // Resizes the cache
        void Resize(const size_t newSize, const bool sizeInMB = true) noexcept;
        
        // Releases the cache, deinitialise everything and free the memory
        void Release() noexcept;
    };

} // end namespace Core