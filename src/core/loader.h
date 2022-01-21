// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#pragma once

#include "GL/glew.h"

#include "OpenEXR/ImfRgbaFile.h"

#include <thread>
#include <condition_variable>
#include <string>
#include <vector>
#include <stdlib.h>
#include <filesystem>

#include "cache.h"
#include "media.h"
#include "utils/filesystem_utils.h"
#include "utils/memory/utils.h"

// A Loader is just a file loader that has a cache and interacts with a player
// There are a many loaders as there are players
// The loader is mostly used to interface the cache with the player, as it passes which image to load to the cache

namespace Core
{
	struct Loader
	{
		tsl::robin_map<uint16_t, Media*> m_Medias; // Contains the medias

		std::vector<std::thread> m_Workers; // Contains workers that are used to load the different images of the media being displayed

		std::condition_variable m_CondVar;

		std::mutex m_Mutex;

		ImageCache* m_Cache = nullptr;

		Logger* m_Logger = nullptr;

		Profiler* m_Profiler = nullptr;

		ImVec2 m_Range = ImVec2(0, 0);

		uint32_t m_BgLoadFrameIndex = 0; // Frame from where to start the bg loading

		uint32_t m_CacheSizeMB = 0;

		int m_CacheMode = 0; // 0 : minimal, 1 : manual, 2 : smart

		uint16_t m_MediaCount = 0;

		uint8_t m_CacheMaxRamToUse = 50;

		uint8_t m_OpenEXRThreads = 8;

		uint8_t m_BgLoadChunkSize = 4; // Number of images to load in the cache at the same time in the background

		bool m_AutoDetectFileSequence = true;
		bool m_HasBeenInitialized = false;
		bool m_IsWorking = false; // State for the worker thread doing the load job
		bool m_NeedBgLoad = false; // Notify the background loader we need it to work
		bool m_StopBgLoad = false; // Stop the background loader

		// Simple constructor to initialize the logger/profiler
		Loader(Logger* logger, Profiler* profiler);

		// Initializes the loader
		void Initialize(const uint8_t cacheMode, const size_t cacheSize = 0, const bool autodetect = true) noexcept;

		// A few inline methods to get/set states on different attributes/objects of the loader
		OV_FORCEINLINE void SetRange(const ImVec2& range) noexcept { this->m_Range = range; }

		OV_FORCEINLINE uint16_t GetMediaCount() const noexcept { return this->m_MediaCount; }

		OV_FORCEINLINE void* GetImageCachePtrFromIndex(const uint32_t cacheIndex) const noexcept { return this->m_Cache->m_Items[cacheIndex].m_DataPtr; }

		OV_FORCEINLINE void SetOpenExrThreadCount(const uint8_t threadCount) noexcept { this->m_OpenEXRThreads = threadCount; }

		// Loads a media into the loader
		int32_t Load(const std::string& mediaPath) noexcept;

		// Returns a pointer to the image corresponding to the current frame
		Image* GetImage(const uint32_t mediaId, const uint32_t frameIndex) noexcept;

		// Return a pointer to the media corresponding to the index
		Media* GetMedia(const uint32_t mediaId) noexcept;

		// Loads an image to display it. If the cache is enabled, it will load it in the cache, otherwise loads it on the fly
		void LoadImageToCache(const uint32_t mediaId, const uint32_t frameIndex, const bool force = false) noexcept;

		// Loads a sequence of images in the cache to display it, starting from the given index until 
		// it reaches startIndex + size
		// If the given size is 0, it will load as much images as the cache can handle
		void LoadSequenceToCache(const uint32_t mediaId, const uint32_t startIndex, const uint32_t size = 0) noexcept;

		// Resizes the cache depending on the cache mode
		void ResizeCache(const uint64_t size, const bool sizeInMB = false) noexcept;

		// Background loading function that loads the images to the cache as a background task
		void BackgroundLoad() noexcept;

		// Launch the worker that loads images to the cache in background
		void LaunchCacheLoader() noexcept;

		// Stop the worker that loads images to the cache in background
		void StopCacheLoader() noexcept;

		// Join all the threads launched during cache usage to load images in the background
		void JoinWorkers() noexcept;

		// Deallocate resources and release the loader
		void Release() noexcept;
	};
} // End namespace Core