// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "cache.h"

namespace Core
{
    void ImageCache::Initialize(const size_t size, Logger* logger, const bool sizeInMB) noexcept
    {
        this->m_HasBeenInitialized = true;
        this->m_Capacity = 0;

        // Size needs to be specified in MB 
        this->m_BytesCapacity = sizeInMB ? size * 1000000 : size;
        this->m_Logger = logger;

        const char* minimalCacheMsg = sizeInMB ? " " : " minimal ";

        this->m_Logger->Log(LogLevel_Diagnostic, "[CACHE] : Initializing%sImage Cache (%.2f MB)", minimalCacheMsg,
                                                                                                  sizeInMB ? 
                                                                                                  static_cast<float>(size) :
                                                                                                  static_cast<float>(size) / 1000000.0f);

        this->m_MemoryArena = OvAlloc(this->m_BytesCapacity, 32);
    }

    uint32_t ImageCache::Add(Image* image, const uint32_t mediaId) noexcept
    {
        std::unique_lock<std::mutex> addLock(this->m_Mtx);

        const uint64_t imgSize = image->m_Xres * image->m_Yres * (image->m_Channels > 4 ? 4 : image->m_Channels);
        const uint64_t imgByteSize = image->m_Stride;

        if (imgByteSize > this->m_BytesCapacity)
        {
            this->Resize(imgByteSize, false);
        }

        // The image we want to store in cache fits, lets give it a new address
        if ((this->m_BytesSize + imgByteSize) <= this->m_BytesCapacity)
        {
            // The new cache address corresponds to the current byte size
            const uint64_t cacheOffset = this->m_BytesSize;

            // Calculate new image address
            void* newImgAddress = static_cast<char*>(this->m_MemoryArena) + cacheOffset;

            // Add the image to the current items
            this->m_Items.emplace(this->m_CurrentIndex, ImageCacheItem(image, newImgAddress, imgByteSize, imgSize, mediaId));

            // Set the index on the image (to notify it has been cached, and to be able to retrieve the address with
            // the index only)
            image->m_CacheIndex = this->m_CurrentIndex;

            this->m_Logger->Log(LogLevel_Debug, "[CACHE] : Added image [%s] to the Image Cache at index [%d]", image->m_Path.c_str(), this->m_CurrentIndex);

            // Update cache size
            this->m_BytesSize += imgByteSize;
            this->m_Logger->Log(LogLevel_Debug, "[CACHE] : Update cache size : %f MB (Capacity : %f MB)", static_cast<float>(this->m_BytesSize) / 1000000.0f,
                                                                                                          static_cast<float>(this->m_BytesCapacity) / 1000000.0f);
            
            ++this->m_CurrentIndex;
            ++this->m_Size;

            addLock.unlock();

            return image->m_CacheIndex;
        }
        // The image we want to store does not fit in the cache. We will release images until
        // we have enough space to store the image
        else
        {
            const uint32_t currentIdx = this->m_CurrentTraversingIndex == 0 ? 0 : this->m_CurrentTraversingIndex % this->m_Size;
            const uint32_t oldSize = this->m_Size;
            uint32_t traversingIdx = currentIdx + 1;
            uint64_t cleanedByteSize = 0;
            uint32_t cleanIndex = 1;
            void* cleanAddress = nullptr;

            while (true)
            {
                traversingIdx = traversingIdx % (oldSize + 1) == 0 ? 1 : traversingIdx;
                ImageCacheItem tmpImgCacheItem = this->m_Items[traversingIdx];

                // bool foundImageCacheItem = this->m_Items.find(traversingIdx) != this->m_Items.end();
                const bool imageFitsInCache = imgByteSize <= (this->m_BytesCapacity - (this->m_BytesSize - tmpImgCacheItem.m_Stride));

                // We found an image that was the same size (or larger) or we have enough space in the cache to load the frame at the current index
                // We remove it from the cache and use its memory to store our new image
                if(imgByteSize <= tmpImgCacheItem.m_Stride || imageFitsInCache)
                {
                    // Notify that this image is not in cache anymore and update the cache infos
                    tmpImgCacheItem.m_Image->m_CacheIndex = 0;
                    
                    this->m_BytesSize -= tmpImgCacheItem.m_Stride;

                    const std::string removedImagePath = tmpImgCacheItem.m_Image->m_Path;

                    void* address = tmpImgCacheItem.m_DataPtr;

                    if (imageFitsInCache)
                    {
                        // Special case where we emptied all the cache but we are not at index 1, so we kind of flush the cache
                        // and add a new item at the index 1
                        if (this->m_Items.find(1) == this->m_Items.end())
                        {
                            this->m_Items.erase(traversingIdx);
                            address = this->m_MemoryArena;
                            traversingIdx = 1;
                        }
                    }

                    this->m_Items[traversingIdx] = ImageCacheItem(image, address, imgByteSize, imgSize, mediaId);

                    // Update cache infos
                    this->m_BytesSize += imgByteSize;

                    // Set the index on the image, to notify it has been loaded into the cache
                    image->m_CacheIndex = traversingIdx;
                    this->m_CurrentTraversingIndex = traversingIdx;
                    this->m_CurrentIndex = traversingIdx + 1;

                    this->m_Logger->Log(LogLevel_Debug, "[CACHE] : Removed image [%s] at index [%d] and loaded a new one [%s]", removedImagePath.c_str(), 
                                                                                                                                traversingIdx,
                                                                                                                                image->m_Path.c_str());
                    this->m_Logger->Log(LogLevel_Debug, "[CACHE] : Update cache size : %f MB (Capacity : %f MB)", static_cast<float>(this->m_BytesSize) / 1000000.0f,
                                                                                                                  static_cast<float>(this->m_BytesCapacity) / 1000000.0f);

                    addLock.unlock();
                    
                    return traversingIdx;
                }
                else
                {
                    // Begin the cleanup, get the address of the first released image
                    // and idem for the index
                    if (cleanedByteSize == 0) 
                    {
                        cleanAddress = tmpImgCacheItem.m_DataPtr;
                        cleanIndex = traversingIdx;
                    }

                    if(imgByteSize <= cleanedByteSize || imgByteSize <= this->m_BytesSize)
                    {
                        this->m_Items[cleanIndex] = ImageCacheItem(image, cleanAddress, imgSize, imgByteSize, mediaId);

                        image->m_CacheIndex = cleanIndex;   
                        this->m_CurrentTraversingIndex = cleanIndex;
                        this->m_CurrentIndex = traversingIdx + 1;

                        this->m_Logger->Log(LogLevel_Debug, "[CACHE] : Loaded image [%s] at index [%d]", image->m_Path.c_str(), traversingIdx);
                        this->m_Logger->Log(LogLevel_Debug, "[CACHE] : Update cache size : %f MB (Capacity : %f MB)", static_cast<float>(this->m_BytesSize) / 1000000.0f,
                                                                                                                      static_cast<float>(this->m_BytesCapacity) / 1000000.0f);

                        addLock.unlock();

                        return cleanIndex;
                    }
                    else
                    {
                        // Notify that we are releasing this image from the cache
                        tmpImgCacheItem.m_Image->m_CacheIndex = 0;

                        this->m_Items.erase(traversingIdx);
                        this->m_Size -= 1;

                        this->m_BytesSize -= tmpImgCacheItem.m_Stride;
                        cleanedByteSize += tmpImgCacheItem.m_Stride;

                        this->m_Logger->Log(LogLevel_Debug, "[CACHE] : Removed image [%s] at index [%d]", tmpImgCacheItem.m_Image->m_Path.c_str(), traversingIdx);
                        this->m_Logger->Log(LogLevel_Debug, "[CACHE] : Update cache size : %f MB (Capacity : %f MB)", static_cast<float>(this->m_BytesSize) / 1000000.0f,
                                                                                                                      static_cast<float>(this->m_BytesCapacity) / 1000000.0f);
                    }
                }

                ++traversingIdx;
            }
        }
    }

    void ImageCache::Remove(const uint32_t index) noexcept
    {
        this->m_Items[index].m_Image->m_CacheIndex = 0;
        this->m_BytesSize -= this->m_Items[index].m_Stride;

        --this->m_CurrentIndex;
        --this->m_Size;

        this->m_Logger->Log(LogLevel_Debug, "[CACHE] : Removed image at index [%d]", index);
    }

    void ImageCache::Flush() noexcept
    {
        for (auto& [index, cacheItem] : this->m_Items)
        {
            cacheItem.m_Image->m_CacheIndex = 0;
        }

        this->m_Items.clear();
        this->m_Size = 0;
        this->m_BytesSize = 0;
        this->m_CurrentIndex = 1;
        this->m_CurrentTraversingIndex = 0;

        this->m_Logger->Log(LogLevel_Diagnostic, "[CACHE] : Image Cache flushed");
    }
    
    void ImageCache::Resize(const size_t newSize, const bool sizeInMB) noexcept
    {
        const uint64_t newSizeBytes = sizeInMB ? newSize * 1000000 : newSize + 1000000; // We add 1 MB to be sure it fits in case of rounding/error

        void* oldAddress = this->m_MemoryArena;
        
        void* tmpMemArena = OvAlloc(newSizeBytes, 32);
        
        memmove(tmpMemArena, this->m_MemoryArena, this->m_BytesSize);
        
        OvFree(this->m_MemoryArena);

        this->m_MemoryArena = tmpMemArena;
        
        this->m_BytesCapacity = newSizeBytes;

        for (auto it = this->m_Items.begin(); it != this->m_Items.end(); it++)
        {
            ImageCacheItem* item = &this->m_Items[it.key()];

            const uint64_t cacheOffset = static_cast<char*>(item->m_DataPtr) - static_cast<char*>(oldAddress);

            item->m_DataPtr = static_cast<char*>(this->m_MemoryArena) + cacheOffset;
        }

        // this->m_Items.clear();
        // this->m_Size = 0;
        // this->m_BytesSize = 0;
        // this->m_CurrentIndex = 1;
        this->m_CurrentTraversingIndex = 0;

        this->m_Logger->Log(LogLevel_Diagnostic, "[CACHE] : Image Cache resized (%.2f MB)", sizeInMB ? 
                                                                                       static_cast<float>(newSize) :
                                                                                       static_cast<float>(newSize) / 1000000.0f);
    }
    
    void ImageCache::Release() noexcept
    {
        if(this->m_MemoryArena != nullptr) 
        {
            OvFree(this->m_MemoryArena);
            this->m_MemoryArena = nullptr;
        }

        for (auto& [index, cacheItem] : this->m_Items)
        {
            cacheItem.m_Image->m_CacheIndex = 0;
        }

        this->m_Items.clear();
        this->m_Size = 0;
        this->m_BytesSize = 0;
        this->m_CurrentIndex = 1;
        this->m_CurrentTraversingIndex = 0;

        this->m_HasBeenInitialized = false;

        // Logger is null if cache has not been initialized
        if (this->m_Logger != nullptr) this->m_Logger->Log(LogLevel_Diagnostic, "[CACHE] : Releasing cache");
    }
} // end namespace Core