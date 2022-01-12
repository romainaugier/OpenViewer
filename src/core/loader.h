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

// WinUser.h has a LoadImage definition
#ifdef _WIN32
#undef LoadImage
#endif

// A Loader is just a file loader that has a cache and interacts with a player
// There are a many loaders as there are players
// The loader is mostly used to interface the cache with the player, as it passes which image to load to the cache

namespace Core
{
	struct Loader
	{
		std::vector<Media> m_Medias; // Contains the medias

		std::vector<std::thread> m_Workers; // Contains workers that are used to load the different images of the media being displayed

		std::condition_variable m_CondVar;

		std::mutex m_Mutex;

		ImageCache* m_Cache = nullptr;

		Logger* m_Logger = nullptr;

		Profiler* m_Profiler = nullptr;

		ImVec2 m_Range = ImVec2(0, 0);

		uint32_t m_BgLoadFrameIndex = 0; // Frame from where to start the bg loading

		uint32_t m_CacheSizeMB = 0;

		uint16_t m_MediaCount = 0;

		uint8_t m_BgLoadChunkSize = 4; // Number of images to load in the cache at the same time in the background

		bool m_AutoDetectFileSequence = true;
		bool m_UseCache = false;
		bool m_HasBeenInitialized = false;
		bool m_IsWorking = false; // State for the worker thread doing the load job
		bool m_NeedBgLoad = false; // Notify the background loader we need it to work
		bool m_StopBgLoad = false; // Stop the background loader

		// Simple constructor to initialize the logger/profiler
		Loader(Logger* logger, Profiler* profiler);

		// Initializes the loader
		void Initialize(const bool useCache, const size_t cacheSize = 0) noexcept;

		// A few inline methods to get/set states on different attributes/objects of the loader
		// Sets a media active
		OV_FORCEINLINE void SetMediaActive(const uint32_t mediaId) noexcept { this->m_Medias[mediaId].SetActive(); }

		OV_FORCEINLINE void SetMediaInactive(const uint32_t mediaId) noexcept { this->m_Medias[mediaId].SetInactive(); }
		
		OV_FORCEINLINE void SetAllMediasInactive() noexcept { for (auto& media : this->m_Medias) media.SetInactive(); }

		OV_FORCEINLINE void SetRange(const ImVec2& range) noexcept { this->m_Range = range; }

		OV_FORCEINLINE uint16_t GetMediaCount() const noexcept { return this->m_MediaCount; }

		// Loads a media into the loader
		void Load(const std::string& mediaPath) noexcept;

		// Returns a pointer to the image corresponding to the current frame
		Image* GetImage(const uint32_t frameIndex) noexcept;

		// Return a pointer to the media corresponding to the index
		Media* GetMedia(const uint32_t mediaId) noexcept;
		
		// Loads an image to display it. If the cache is enabled, it will load it in the cache, otherwise loads it on the fly
		void LoadImageToCache(const uint32_t index) noexcept;

		// Loads a sequence of images in the cache to display it, starting from the given index until 
		// it reaches startIndex + size
		// If the given size is 0, it will load as much images as the cache can handle
		void LoadSequenceToCache(const uint32_t startIndex, const uint32_t size = 0) noexcept;

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