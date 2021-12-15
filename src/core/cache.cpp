// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "cache.h"

namespace Core
{
    void ImageCache::Initialize(const size_t size, Logger* logger) noexcept
    {
        this->m_Capacity = size;
        // Size needs to be specified in MB 
        this->m_BytesCapacity = size * 1000000;
        this->m_Logger = logger;

        this->m_Logger->Log(LogLevel_Debug, "[CACHE] : Initializing Image Cache (%d MB)", size);

        this->m_MemoryArena = OvAlloc(this->m_BytesCapacity, 32);
    }

    uint16_t ImageCache::Add(Image* image) noexcept
    {
        this->m_Mtx.lock();

        const uint64_t imgSize = image->m_Xres * image->m_Yres * image->m_Channels;
        const uint64_t imgByteSize = imgSize * image->m_Format;

        // The image we want to store in cache fits, lets give it a new address
        if ((this->m_BytesSize + imgByteSize) < this->m_BytesCapacity)
        {
            // Update cache size
            this->m_BytesSize += imgByteSize;

            // Calculate new image address
            void* newImgAddress = static_cast<uint8_t*>(this->m_MemoryArena) + this->m_BytesSize + imgByteSize;

            // Add the image to the current items
            this->m_Items.emplace(this->m_CurrentIndex, ImageCacheItem(image, newImgAddress, imgSize, imgByteSize));

            // Set the index on the image (to notify it has been cached, and to be able to retrieve the address with
            // the index only)
            image->m_CacheIndex = this->m_CurrentIndex;

            this->m_CurrentIndex += 1;
            this->m_Size += 1;

            this->m_Logger->Log(LogLevel_Debug, "[CACHE] : Added image to the Image Cache at index %d", this->m_CurrentIndex);

            this->m_Mtx.unlock();

            return this->m_CurrentIndex - 1;
        }
        // The image we want to store does not fit in the cache. We will release images until
        // we have enough space to store the image
        else
        {
            uint16_t traversingIdx = 1;
            uint64_t cleanedByteSize = 0;
            uint16_t cleanIndex = 1;
            void* cleanAddress = nullptr;

            while (true)
            {
                ImageCacheItem tmpImgCacheItem = this->m_Items[traversingIdx];

                // We found an image that was the same size (or larger)
                // We remove it from the cache and use its memory to store our new image
                if(imgByteSize <= tmpImgCacheItem.m_Stride)
                {
                    // Notify that this image is not in cache anymore and update the cache infos
                    tmpImgCacheItem.m_Image->m_CacheIndex = 0;
                    this->m_BytesSize -= tmpImgCacheItem.m_Stride;

                    void* address = tmpImgCacheItem.m_Ptr;

                    this->m_Items[traversingIdx] = ImageCacheItem(image, address, imgSize, imgByteSize);

                    // Set the index on the image, to notify it has been loaded into the cache
                    image->m_CacheIndex = traversingIdx;

                    this->m_Logger->Log(LogLevel_Debug, "[CACHE] : Removed image at index [%d] and loaded a new one", traversingIdx);

                    this->m_Mtx.unlock();
                    
                    return traversingIdx;
                }
                else
                {
                    // Begin the cleanup, get the address of the first released image
                    // and idem for the index
                    if (cleanedByteSize == 0) 
                    {
                        cleanAddress = tmpImgCacheItem.m_Ptr;
                        cleanIndex = traversingIdx;
                    }

                    if(imgByteSize <= cleanedByteSize)
                    {
                        this->m_Items[cleanIndex] = ImageCacheItem(image, cleanAddress, imgSize, imgByteSize);
                        image->m_CacheIndex = cleanIndex;   

                        this->m_Logger->Log(LogLevel_Debug, "[CACHE] : Loaded image at index [%d]", traversingIdx);

                        this->m_Mtx.unlock();

                        return cleanIndex;
                    }
                    else
                    {
                        // Notify that we are releasing this image from the cache
                        tmpImgCacheItem.m_Image->m_CacheIndex = 0;

                        this->m_BytesSize -= tmpImgCacheItem.m_Stride;
                        cleanedByteSize += tmpImgCacheItem.m_Stride;

                        this->m_Logger->Log(LogLevel_Debug, "[CACHE] : Removed image at index [%d]", traversingIdx);
                    }
                }


                ++traversingIdx;
            }
        }
    }

    void ImageCache::Remove(const uint16_t index) noexcept
    {
        // TODO
    }
    
    void ImageCache::Resize(const size_t newSize) noexcept
    {
        // Size is given in MB
        
        const uint64_t newSizeBytes = newSize * 1000000;
        
        if(newSizeBytes < this->m_BytesCapacity) return;
        else
        {
            OvFree(this->m_MemoryArena);

            void* newPtr = OvAlloc(newSizeBytes, 32);
            memmove(newPtr, this->m_MemoryArena, this->m_BytesSize);
            this->m_MemoryArena = newPtr;
            this->m_BytesCapacity = newSizeBytes;
        }

        this->m_Logger->Log(LogLevel_Debug, "[CACHE] : Image Cache resized (%d MB)", newSize);
    }
    
    void ImageCache::Release() noexcept
    {
        if(this->m_MemoryArena != nullptr) 
        {
            OvFree(this->m_MemoryArena);
            this->m_MemoryArena = nullptr;
        }
    }
} // end namespace Core