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
		std::vector<Image> m_Images; // Contains the image seq that will be played in the player which the loader is attached to

		std::vector<std::thread> m_Workers; // Contains workers that are used to load the different images

		std::condition_variable m_CondVar;

		ImageCache* m_Cache = nullptr;

		Logger* m_Logger = nullptr;

		Profiler* m_Profiler = nullptr;

		uint16_t m_ImageCount = 0;

		bool m_UseCache = false;
		bool m_HasBeenInitialized = false;
		bool m_IsWorking = false; // State for the worker thread doing the load job

		// Simple constructor to initialize the logger/profiler
		Loader(Logger* logger, Profiler* profiler);

		// Initializes the loader with a directory path and the cache if used. The cache size needs to be expressed in MB
		void Initialize(const std::string& directoryPath, const bool useCache, const size_t cacheSize) noexcept;
		
		// Initializes the loader with a single image, and as it is a single image it will not be cached
		void Initialize(const std::string& imagePath) noexcept;
		
		// Loads an image. If the cache is enabled, it will load it in the cache, otherwise loads it on the fly
		void LoadImage(const uint16_t index) noexcept;

		// Loads a sequence of images in the cache, starting from the given index until 
		// it reaches startIndex + size
		// If the given size is 0, it will load as much images as the cache can handle
		void LoadSequence(const uint16_t startIndex, const uint16_t size = 0) noexcept;

		// Deallocate resources and release the loader
		void Release() noexcept;
	};
} // End namespace Core