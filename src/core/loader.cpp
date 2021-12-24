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

	void Loader::Initialize(const bool useCache, const size_t cacheSize) noexcept
	{
		this->m_Logger->Log(LogLevel_Debug, "[LOADER] : Initializing loader");

		this->m_HasBeenInitialized = true;

		// Allocate the cache (if used)
		if (useCache)
		{
			this->m_UseCache = true;
			this->m_Cache->Initialize(cacheSize, this->m_Logger);
		}
	}

	void Loader::Load(const std::string& mediaPath) noexcept
	{
		this->m_Logger->Log(LogLevel_Debug, "[LOADER] : Loading media %s", mediaPath.c_str());

		if (std::filesystem::is_directory(mediaPath))
		{
			this->m_Logger->Log(LogLevel_Debug, "[LOADER] : Directory detected, loading image sequence");

			Media newTmpMedia;

			uint32_t itemCount = Utils::FileCountInDirectory(mediaPath);

			newTmpMedia.m_Images.reserve(itemCount);

			for (const auto& item : std::filesystem::directory_iterator(mediaPath))
			{
				if (item.is_regular_file())
				{
					const std::string itemPath = item.path().u8string();

					newTmpMedia.m_Images.emplace_back(itemPath);

					this->m_Logger->Log(LogLevel_Debug, "[LOADER] : Loaded : %s", itemPath.c_str());

					++newTmpMedia.m_Range.y;
				}
				else
				{
					this->m_Logger->Log(LogLevel_Debug, "[LOADER] : File : %s is invalid, discarded", item.path().c_str());
				}
			}

			newTmpMedia.m_Images.shrink_to_fit();

			// Move the newly created media into the media vector of the loader
			this->m_Medias.push_back(std::move(newTmpMedia));
			++this->m_MediaCount;
		}
		else if (std::filesystem::is_regular_file(mediaPath))
		{
			this->m_Logger->Log(LogLevel_Debug, "[LOADER] : Single image detected, loading single image");

			Media newTmpMedia;

			newTmpMedia.m_Images.emplace_back(mediaPath);

			this->m_Logger->Log(LogLevel_Debug, "[LOADER] : Loaded : %s", mediaPath.c_str());

			// Move the media into the media vector of the loader
			this->m_Medias.push_back(std::move(newTmpMedia));
			++this->m_MediaCount;
		}
		else
		{
			this->m_Logger->Log(LogLevel_Error, "[LOADER] : Path : %s is invalid", mediaPath.c_str());
		}
	}

	Image* Loader::GetImage(const uint32_t frameIndex) noexcept
	{
		for (auto& media : this->m_Medias)
		{
			// Skip medias that are not in the timeline
			if (!media.m_IsActive) continue;

			// The image we are looking for is in this media, so load it
			if (media.InRange(frameIndex))
			{
				const uint32_t imageIndex = frameIndex - media.m_TimelineRange.x;
				return &media.m_Images[imageIndex];
			}
		}

		return nullptr;
	}

	void Loader::LoadImageToCache(const uint32_t index) noexcept
	{
		if(this->m_UseCache)
		{
			for (auto& media : this->m_Medias)
			{
				// Skip medias that are not in the timeline
				if (!media.m_IsActive) continue;

				// The image we are looking for is in this media, so load it
				if (media.InRange(index))
				{
					const uint32_t imageIndex = index - media.m_TimelineRange.x;
					const uint32_t cachedIndex = this->m_Cache->Add(&media.m_Images[imageIndex]);

					media.m_Images[imageIndex].Load(this->m_Cache->m_Items[cachedIndex].m_Ptr, this->m_Profiler);

					break;
				}
			}
		}
		else
		{
			for (auto& media : this->m_Medias)
			{
				// Skip medias that are not in the timeline
				if (!media.m_IsActive) continue;

				// The image we are looking for is in this media, so load it
				if (media.InRange(index))
				{
					const uint32_t imageIndex = index - media.m_TimelineRange.x;

					media.m_Images[imageIndex].Load(this->m_Cache->m_Items[1].m_Ptr, this->m_Profiler);

					break;
				}
			}
		}
	}

	void Loader::LoadSequenceToCache(const uint32_t startIndex, const uint32_t size = 0) noexcept
	{
		
	}

	void Loader::Release() noexcept
	{
		// Release cache
		this->m_Cache->Release();
		delete this->m_Cache;

		// Clear the medias
		for(auto& media : this->m_Medias) 
		{
			media.m_Images.clear();
		}

		// Clear all the vectors
		this->m_Medias.clear();
		this->m_Medias.resize(0);

		this->m_Workers.clear();
		this->m_Workers.resize(0);
	}
}