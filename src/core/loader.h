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

		std::vector<std::thread> m_Workers; // Contains workers that are used to load the different images

		std::condition_variable m_CondVar;

		ImageCache* m_Cache = nullptr;

		Logger* m_Logger = nullptr;

		Profiler* m_Profiler = nullptr;

		bool m_UseCache = false;
		bool m_HasBeenInitialized = false;
		bool m_IsWorking = false; // State for the worker thread doing the load job

		// Simple constructor to initialize the logger/profiler
		Loader(Logger* logger, Profiler* profiler);

		// Initializes the loader
		void Initialize(const bool useCache, const size_t cacheSize) noexcept;

		// Loads a media into the loader
		void Load(const std::string& mediaPath, const bool directory) noexcept;
		
		// Loads an image to display it. If the cache is enabled, it will load it in the cache, otherwise loads it on the fly
		void LoadImageToCache(const uint16_t index) noexcept;

		// Loads a sequence of images in the cache to display it, starting from the given index until 
		// it reaches startIndex + size
		// If the given size is 0, it will load as much images as the cache can handle
		void LoadSequenceToCache(const uint16_t startIndex, const uint16_t size = 0) noexcept;

		// Deallocate resources and release the loader
		void Release() noexcept;
	};
} // End namespace Core