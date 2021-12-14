// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#pragma once

#include "OpenImageIO/imagecache.h"
#include "GL/glew.h"

#include "OpenEXR/ImfImage.h"
#include "OpenEXR/ImfRgbaFile.h"

#include <thread>
#include <string>
#include <vector>
#include <stdlib.h>
#include <filesystem>

#include "cache.h"

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

		ImageCache* m_Cache;

		bool m_UseCache = false;

		// Initializes the loader with a directory path and the cache size
		void Initialize(const std::string& directoryPath, const size_t cacheSize) noexcept;
		
		// Initializes the loader with a single image, and as it is a single image it will not be cached
		void Initialize(const std::string& imagePath) noexcept;
		
		// Loads an image. If the cache is enabled, it will load it in the cache, otherwise loads it on the fly
		void LoadImage(const uint16_t index) noexcept;

		// Loads as much image as it can in the cache
		void LoadSequence() noexcept;

		// Deallocate resources and release the loader
		void Release() noexcept;
	};
} // End namespace Core