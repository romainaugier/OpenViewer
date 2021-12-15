// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "loader.h"

namespace Core
{
	Loader::Loader(Logger* logger, Profiler* profiler)
	{
		this->m_Logger = logger;
		this->m_Profiler = profiler;

		this->m_Cache = new ImageCache;
	}

	void Loader::Initialize(const std::string& directoryPath, const bool useCache, const size_t cacheSize) noexcept
	{
		this->m_Logger->Log(LogLevel_Debug, "[LOADER] : Initializing directory loader with %s", directoryPath.c_str());

		this->m_HasBeenInitialized = true;

		// Check if the given path is a directory, otherwise return an error
		if(!std::filesystem::is_directory(directoryPath))
		{
			this->m_Logger->Log(LogLevel_Error, "[LOADER] : Failed to initialize loader. %s is not a directory or does not exist", directoryPath.c_str());
			return;
		}

		const uint16_t itemCount = static_cast<uint16_t>(Utils::FileCountInDirectory(directoryPath));
		this->m_Images.reserve(itemCount);

		// We'll check the biggest size of the sequence so if we don't use the cache sequence,
		// enough space will be allocated in the single frame cache
		uint64_t maxImageSize = 0;

		// Loop through the directory and load every file there is in the directory
		// TODO : check if the file is an image file by checking the extension
		for(const auto& p : std::filesystem::directory_iterator(directoryPath))
		{
			const std::string filePath = p.path().u8string();

			if (std::filesystem::is_regular_file(filePath))
			{
				++this->m_ImageCount;
				this->m_Images.emplace_back(filePath);

				const uint64_t curImgSize = this->m_Images[this->m_ImageCount - 1].m_Stride;

				maxImageSize = curImgSize > maxImageSize ? curImgSize : maxImageSize;

				this->m_Logger->Log(LogLevel_Debug, "[LOADER] : Loaded %s", filePath.c_str());
			}
			else
			{
				this->m_Logger->Log(LogLevel_Debug, "[LOADER] : File %s is invalid, discarding it", filePath.c_str());
			}
		}

		// Allocate the cache (if used)
		if (useCache)
		{
			this->m_UseCache = true;
			this->m_Cache->Initialize(cacheSize, this->m_Logger);
		}
		// The cache is not used (for sequence caching), but we will use it to
		// allocate the cache for a single frame, and if the sequence has images
		// of different size, we make sure to allocate to the size of the biggest
		// image so there is no issue
		else
		{
			this->m_UseCache = false;
			this->m_Cache->Initialize(maxImageSize, this->m_Logger);
		}

		// Load the first image
		this->LoadImage(0);
	}

	void Loader::Initialize(const std::string& imagePath) noexcept
	{
		this->m_Logger->Log(LogLevel_Debug, "[LOADER] : Initializing image loader with %s", imagePath.c_str());

		this->m_HasBeenInitialized = true;

		// Check if the given path is a valid file and exists
		if(!std::filesystem::is_regular_file(imagePath))
		{
			this->m_Logger->Log(LogLevel_Debug, "[LOADER] : Failed to initialize loader. %s is not a valid file or does not exist", imagePath.c_str());
			return;
		}

		++this->m_ImageCount;
		this->m_Images.emplace_back(imagePath);

		const uint64_t curImgSize = this->m_Images[this->m_ImageCount - 1].m_Stride;

		this->m_Logger->Log(LogLevel_Debug, "[LOADER] : Loaded %s", imagePath.c_str());

		// Allocate the cache for the single image
		this->m_UseCache = false;
		this->m_Cache->Initialize(curImgSize, this->m_Logger);

		// Load the image
		this->LoadImage(0);
	}

	void Loader::LoadImage(const uint16_t index) noexcept
	{
		if(this->m_UseCache)
		{
			const uint16_t cachedIndex = this->m_Cache->Add(&this->m_Images[index]);

			this->m_Images[index].Load(this->m_Cache->m_Items[cachedIndex].m_Ptr, this->m_Profiler);
		}
		else
		{
			this->m_Images[index].Load(this->m_Cache->m_MemoryArena, this->m_Profiler);
		}
	}

	void Loader::Release() noexcept
	{
		// Release cache
		this->m_Cache->Release();
		delete this->m_Cache;

		// Clear all the vectors
		this->m_Images.clear();
		this->m_Images.resize(0);

		this->m_Workers.clear();
		this->m_Workers.resize(0);
	}
}