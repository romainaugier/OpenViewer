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
		const std::string cleanMediaPath = Utils::CleanOSPath(mediaPath);

		this->m_Logger->Log(LogLevel_Debug, "[LOADER] : Loading media %s", cleanMediaPath.c_str());

		if (std::filesystem::is_directory(cleanMediaPath))
		{
			this->m_Logger->Log(LogLevel_Debug, "[LOADER] : Directory detected, loading image sequence");

			Media newTmpMedia;

			uint32_t itemCount = Utils::FileCountInDirectory(cleanMediaPath);

			uint64_t biggestImageByteSize = 0;

			newTmpMedia.m_Images.reserve(itemCount);

			for (const auto& item : std::filesystem::directory_iterator(cleanMediaPath))
			{
				if (item.is_regular_file())
				{
					std::string itemPath = item.path().u8string();

					Utils::CleanOSPath(itemPath);

					newTmpMedia.m_Images.emplace_back(itemPath);

					this->m_Logger->Log(LogLevel_Debug, "[LOADER] : Loaded : %s", itemPath.c_str());

					// If images in the sequence are of different sizes, get the biggest image of the sequence to 
					// update the cache
					biggestImageByteSize = biggestImageByteSize < newTmpMedia.m_Images[newTmpMedia.m_Range.y].m_Stride ? 
																  newTmpMedia.m_Images[newTmpMedia.m_Range.y].m_Stride : 
																  biggestImageByteSize;

					++newTmpMedia.m_Range.y;
				}
				else
				{
					this->m_Logger->Log(LogLevel_Debug, "[LOADER] : File : %s is invalid, discarded", item.path().c_str());
				}
			}

			// Initialize the cache if it has not been, else resize the cache if needed (by cache I mean the minimal cache needed to display 
			// at least one image). We resize the minimal cache to the size of the biggest image in all the medias. It avoids allocations/deallocations
			// during reading
			if (!this->m_UseCache)
			{
				if (this->m_Cache->m_HasBeenInitialized)
				{
					// Cache will detect automatically if it needs to be resized
					this->m_Cache->Resize(biggestImageByteSize, false);
				}
				else
				{
					this->m_Cache->Initialize(biggestImageByteSize, this->m_Logger, false);
				}
			}

			newTmpMedia.m_Images.shrink_to_fit();

			// Move the newly created media into the media vector of the loader
			this->m_Medias.push_back(std::move(newTmpMedia));
			++this->m_MediaCount;
		}
		else if (std::filesystem::is_regular_file(cleanMediaPath))
		{
			this->m_Logger->Log(LogLevel_Debug, "[LOADER] : Single image detected, loading single image");

			Media newTmpMedia;

			newTmpMedia.m_Images.emplace_back(cleanMediaPath);

			this->m_Logger->Log(LogLevel_Debug, "[LOADER] : Loaded : %s", cleanMediaPath.c_str());

			// Same as in the sequence loading
			if (!this->m_UseCache)
			{
				const uint64_t loadedImgByteSize = newTmpMedia.m_Images[0].m_Stride;

				if (this->m_Cache->m_HasBeenInitialized)
				{
					// Cache will detect automatically if it needs to be resized
					this->m_Cache->Resize(loadedImgByteSize, false);
				}
				else
				{
					this->m_Cache->Initialize(loadedImgByteSize, this->m_Logger, false);
				}
			}

			// Move the media into the media vector of the loader
			this->m_Medias.push_back(std::move(newTmpMedia));
			++this->m_MediaCount;
		}
		else
		{
			this->m_Logger->Log(LogLevel_Error, "[LOADER] : Path : %s is invalid", cleanMediaPath.c_str());
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
		const Image* tmpImg = this->GetImage(index);

		if (tmpImg->m_CacheIndex > 0) return;

		if (this->m_UseCache)
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

					// if (this->m_Cache->m_Size > 0) this->m_Cache->Remove(1);

					this->m_Cache->Add(&media.m_Images[imageIndex]);

					media.m_Images[imageIndex].Load(this->m_Cache->m_Items[1].m_Ptr, this->m_Profiler);

					break;
				}
			}
		}
	}

	void Loader::LoadSequenceToCache(const uint32_t startIndex, const uint32_t size) noexcept
	{
		uint32_t endIndex = size;
		
		if (size == 0)
		{
			uint32_t index = startIndex;
			uint64_t accumulatedByteSize = 0;
			endIndex = startIndex;

			while (true)
			{
				const Image* tmpImage = this->GetImage(index);

				if (tmpImage == nullptr) break;

				accumulatedByteSize += tmpImage->m_Stride;

				if (accumulatedByteSize >= this->m_Cache->m_BytesCapacity) break;

				++endIndex;
				++index;
			}
		}

		for (uint32_t i = startIndex; i < (endIndex - 1); i++)
		{
			this->LoadImageToCache(i);
		}
	}

	void Loader::BackgroundLoad() noexcept
	{
		while(true)
		{
			std::unique_lock<std::mutex> lock(this->m_Mutex);

			this->m_CondVar.wait(lock, [this] { return this->m_NeedBgLoad || this->m_StopBgLoad; });

			if (this->m_StopBgLoad) 
			{
				this->m_Logger->Log(LogLevel_Debug, "[LOADER] : Stopping background loader");
				this->m_IsWorking = false;
				break;
			}
			else if (this->m_NeedBgLoad)
			{
				this->LoadSequenceToCache(this->m_BgLoadFrameIndex, this->m_BgLoadChunkSize);

				this->m_NeedBgLoad = false;
			}

			lock.unlock();
			this->m_CondVar.notify_all();
		}
	}

	void Loader::LaunchCacheLoader() noexcept
	{
		if (!this->m_IsWorking)
		{
			this->m_Logger->Log(LogLevel_Debug, "[LOADER] : Starting background loader");

			this->m_Workers.emplace_back(&Loader::BackgroundLoad, this);

			this->m_IsWorking = true;
		}
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

		this->m_Logger->Log(LogLevel_Debug, "[LOADER] : Released loader");
	}
}