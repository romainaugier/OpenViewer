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

	void Loader::Initialize(const uint8_t cacheMode, const size_t cacheSize, const bool autodetect) noexcept
	{
		this->m_Logger->Log(LogLevel_Diagnostic, "[LOADER] : Initializing loader");
		this->m_Logger->Log(LogLevel_Debug, "[LOADER] : Loader OIIO version : %s", Utils::Str::GetOIIOVersionStr().c_str());

		this->m_HasBeenInitialized = true;
		this->m_CacheMode = cacheMode;

		if (cacheMode == 1)
		{
			this->m_CacheMaxSizeMB = cacheSize;
		}
		else if (cacheMode == 2)
		{
			this->m_CacheMaxSizeMB = Utils::BytesToMega(((Utils::GetTotalSystemMemory() * cacheSize) / 100));
		}

		this->m_AutoDetectFileSequence = autodetect;
	}

	int32_t Loader::Load(const std::string& mediaPath) noexcept
	{
		const std::string cleanMediaPath = Utils::Str::CleanOSPath(mediaPath);

		this->m_Logger->Log(LogLevel_Diagnostic, "[LOADER] : Loading media %s", cleanMediaPath.c_str());

		// Load every image there is in a directory
		if (std::filesystem::is_directory(cleanMediaPath))
		{
			// Here, we loop through all the items of the directory and find if there are image sequences that need to be loaded
			
			this->m_Logger->Log(LogLevel_Diagnostic, "[LOADER] : Directory detected");

			tsl::robin_map<std::string, std::string> filesMap;

			for (const auto file : std::filesystem::directory_iterator(cleanMediaPath))
			{
				if (file.is_regular_file())
				{
					std::string path = file.path().u8string();

					Utils::Str::CleanOSPath(path);

					filesMap.emplace(std::make_pair(path, path));
				}
			}

			for (auto it = filesMap.cbegin(); it != filesMap.cend();)
			{
				if (Utils::Fs::IsVideo(it.value()))
				{
					this->m_Logger->Log(LogLevel_Error, "[LOADER] : Videos are not supported for now");
					++it;
				}
				else if (Utils::Fs::IsImage(it.value()))
				{
					Utils::Fs::FileSequence fileSeq;
					Utils::Fs::GetFileSequenceFromFile(fileSeq, it.value());

					if (fileSeq.size() > 1)
					{
						this->m_Logger->Log(LogLevel_Diagnostic, "[LOADER] : File sequence detected");

						std::vector<Image> images;
						images.reserve(fileSeq.size());

						uint32_t imageIndex = 0;

						uint64_t biggestImageByteSize = 0;
						uint64_t totalByteSize = 0;

						for (const auto& fileSeqItem : fileSeq)
						{
							images.emplace_back(fileSeqItem.first);

							this->m_Logger->Log(LogLevel_Debug, "[LOADER] : Loaded %s", fileSeqItem.first.c_str());

							biggestImageByteSize = biggestImageByteSize < images[imageIndex].m_Stride ? 
																			images[imageIndex].m_Stride : 
																			biggestImageByteSize;

							totalByteSize += images[imageIndex].m_Stride;
							
							filesMap.erase(fileSeqItem.first);

							++imageIndex;
						}

						if (Utils::Fs::IsOpenEXR(fileSeq[0].first))
						{
							EXRSequence* newMedia = new EXRSequence(fileSeq[0].first, this->m_MediaCount);
							newMedia->SetID(this->m_MediaCount);
							newMedia->SetImages(images);
							newMedia->SetLayers();
							newMedia->SetRange(ImVec2(0, imageIndex));
							newMedia->SetTotalByteSize(totalByteSize);
							newMedia->SetBiggestImageSize(biggestImageByteSize);

							this->m_Logger->Log(LogLevel_Debug, "Found layers : ");
							for (const auto& layer : newMedia->GetLayers()) 
							{
								this->m_Logger->Log(LogLevel_Debug, "%s (channels : %s)", layer.first.c_str(), layer.second.c_str());
							}

							this->m_Medias[this->m_MediaCount] = newMedia;
						}
						else
						{
							ImageSequence* newMedia = new ImageSequence(fileSeq[0].first, this->m_MediaCount);
							newMedia->SetID(this->m_MediaCount);
							newMedia->SetImages(images);
							newMedia->SetRange(ImVec2(0, imageIndex));
							newMedia->SetTotalByteSize(totalByteSize);
							newMedia->SetBiggestImageSize(biggestImageByteSize);

							this->m_Medias[this->m_MediaCount] = newMedia;
						}
					}
					else
					{
						this->m_Logger->Log(LogLevel_Diagnostic, "[LOADER] : Single image detected");

						std::vector<Image> image;

						image.emplace_back(it.value());

						const uint64_t biggestImageByteSize = image[0].m_Stride;
						const uint64_t totalByteSize = biggestImageByteSize;

						this->m_Logger->Log(LogLevel_Debug, "[LOADER] : Loaded : %s", it.value().c_str());
						
						if (Utils::Fs::IsOpenEXR(it.value()))
						{
							EXRSequence* newMedia = new EXRSequence(it.value(), this->m_MediaCount);
							newMedia->SetID(this->m_MediaCount);
							newMedia->SetImages(image);
							newMedia->SetLayers();
							newMedia->SetRange(ImVec2(0, 1));
							newMedia->SetTotalByteSize(totalByteSize);
							newMedia->SetBiggestImageSize(biggestImageByteSize);

							this->m_Logger->Log(LogLevel_Debug, "[LOADER] : Found layers : ");
							for (const auto& layer : newMedia->GetLayers()) 
							{
								this->m_Logger->Log(LogLevel_Debug, "%s (channels : %s)", layer.first.c_str(), layer.second.c_str());
							}


							this->m_Medias[this->m_MediaCount] = newMedia;
						}
						else
						{
							ImageSequence* newMedia = new ImageSequence(it.value(), this->m_MediaCount);
							newMedia->SetID(this->m_MediaCount);
							newMedia->SetImages(image);
							newMedia->SetRange(ImVec2(0, 1));
							newMedia->SetTotalByteSize(totalByteSize);
							newMedia->SetBiggestImageSize(biggestImageByteSize);

							this->m_Medias[this->m_MediaCount] = newMedia;
						}
					}

					++it;
					++this->m_MediaCount;
				}
			}

			return this->m_MediaCount - 1;
		}

		// Load a given file
		// If the auto detect file sequence is on, it will load the file sequence
		// the given file belongs to (if there is one)
		else if (std::filesystem::is_regular_file(cleanMediaPath))
		{
			uint64_t biggestImageByteSize = 0;
			uint64_t totalByteSize = 0;

			// Loads a video media
			if (Utils::Fs::IsVideo(cleanMediaPath))
			{
				this->m_Logger->Log(LogLevel_Error, "[LOADER] : Videos are not supported for now");
				return -1;
			}
			else if(Utils::Fs::IsImage(cleanMediaPath))
			{	
				if (this->m_AutoDetectFileSequence)
				{
					Utils::Fs::FileSequence fileSeq;
					Utils::Fs::GetFileSequenceFromFile(fileSeq, cleanMediaPath);

					if (fileSeq.size() > 1)
					{
						this->m_Logger->Log(LogLevel_Diagnostic, "[LOADER] : File sequence detected");

						std::vector<Image> images;
						images.reserve(fileSeq.size());

						uint32_t imageIndex = 0;

						for (const auto& fileSeqItem : fileSeq)
						{
							images.emplace_back(fileSeqItem.first);

							this->m_Logger->Log(LogLevel_Debug, "[LOADER] : Loaded %s", fileSeqItem.first.c_str());

							biggestImageByteSize = biggestImageByteSize < images[imageIndex].m_Stride ? 
																		  images[imageIndex].m_Stride : 
																		  biggestImageByteSize;

							totalByteSize += images[imageIndex].m_Stride;

							++imageIndex;
						}

						if (Utils::Fs::IsOpenEXR(cleanMediaPath))
						{
							EXRSequence* newMedia = new EXRSequence(cleanMediaPath, this->m_MediaCount);
						    newMedia->SetID(this->m_MediaCount);
							newMedia->SetImages(images);
							newMedia->SetLayers();
							newMedia->SetRange(ImVec2(0, imageIndex));
							newMedia->SetTotalByteSize(totalByteSize);
							newMedia->SetBiggestImageSize(biggestImageByteSize);

							std::stringstream foundLayers;

							for (const auto& layer : newMedia->GetLayers()) 
							{
								foundLayers << layer.first << " (channels : " << layer.second << ")\n";
							}
							
							this->m_Logger->Log(LogLevel_Debug, "[LOADER] : Found layers : \n%s", foundLayers.str().c_str());

							this->m_Medias[this->m_MediaCount] = newMedia;
						}
						else
						{
							ImageSequence* newMedia = new ImageSequence(cleanMediaPath, this->m_MediaCount);
						    newMedia->SetID(this->m_MediaCount);
							newMedia->SetImages(images);
							newMedia->SetRange(ImVec2(0, imageIndex));
							newMedia->SetTotalByteSize(totalByteSize);
							newMedia->SetBiggestImageSize(biggestImageByteSize);

							this->m_Medias[this->m_MediaCount] = newMedia;
						}
					}
					else
					{
						goto singleimage;
					}
				}
				else
				{
					singleimage:
					
					this->m_Logger->Log(LogLevel_Diagnostic, "[LOADER] : Single image detected");

					std::vector<Image> image;

					image.emplace_back(cleanMediaPath);

					biggestImageByteSize = image[0].m_Stride;
					totalByteSize = biggestImageByteSize;

					this->m_Logger->Log(LogLevel_Debug, "[LOADER] : Loaded : %s", cleanMediaPath.c_str());
					
					if (Utils::Fs::IsOpenEXR(cleanMediaPath))
					{
						EXRSequence* newMedia = new EXRSequence(cleanMediaPath, this->m_MediaCount);
						newMedia->SetID(this->m_MediaCount);
						newMedia->SetImages(image);
						newMedia->SetLayers();
						newMedia->SetRange(ImVec2(0, 1));
						newMedia->SetTotalByteSize(totalByteSize);
						newMedia->SetBiggestImageSize(biggestImageByteSize);

						std::stringstream foundLayers;

						for (const auto& layer : newMedia->GetLayers()) 
						{
							foundLayers << layer.first << " (channels : " << layer.second << ")\n";
						}
						
						this->m_Logger->Log(LogLevel_Debug, "[LOADER] : Found layers : \n%s", foundLayers.str().c_str());

						this->m_Medias[this->m_MediaCount] = newMedia;
					}
					else
					{
						ImageSequence* newMedia = new ImageSequence(cleanMediaPath, this->m_MediaCount);
						newMedia->SetID(this->m_MediaCount);
						newMedia->SetImages(image);
						newMedia->SetRange(ImVec2(0, 1));
						newMedia->SetTotalByteSize(totalByteSize);
						newMedia->SetBiggestImageSize(biggestImageByteSize);

						this->m_Medias[this->m_MediaCount] = newMedia;
					}
				}
			}
			else
			{
				this->m_Logger->Log(LogLevel_Error, "[LOADER] : Invalid file (%s) will not be loaded", mediaPath.c_str());
				return -1;
			}

			++this->m_MediaCount;

			return (this->m_MediaCount - 1);
		}
		else
		{
			this->m_Logger->Log(LogLevel_Error, "[LOADER] : Path : %s is not a valid file or a valid directory", cleanMediaPath.c_str());
			return -1;
		}
	}

	Image* Loader::GetImage(const uint32_t mediaId, const uint32_t frameIndex) noexcept
	{
		return this->m_Medias[mediaId]->GetImage(frameIndex);
	}

	Media* Loader::GetMedia(const uint32_t mediaId) noexcept
	{
		assert(mediaId < this->m_MediaCount);
		
		return this->m_Medias[mediaId];
	}

	void Loader::LoadImageToCache(const uint32_t mediaId, const uint32_t frameIndex, const bool force) noexcept
	{
		const auto imgLoadStart = this->m_Profiler->Start();

		Image* tmpImg = this->GetImage(mediaId, frameIndex);

		if (tmpImg == nullptr || (tmpImg->m_CacheIndex > 0 && !force)) return;

		// If the media is an exr sequence, verify that the current image matches the caracteristics of the selected image layer
		EXRSequence* tmpExrMedia;
		
		if (tmpExrMedia = dynamic_cast<EXRSequence*>(this->GetMedia(mediaId)))
		{
			tmpImg->VerifyChannelSize(tmpExrMedia->GetCurrentChannels());
			tmpExrMedia->SetNumThreads(this->m_OpenEXRThreads);
		}	
		
		const uint32_t cachedIndex = this->m_Cache->Add(tmpImg, mediaId);

		this->GetMedia(mediaId)->LoadImage(frameIndex, this->m_Cache->m_Items[cachedIndex].m_DataPtr);

		const auto imgLoadEnd = this->m_Profiler->End();

		this->m_Profiler->Time("Image Loading Time", imgLoadStart, imgLoadEnd);
	}

	void Loader::LoadSequenceToCache(const uint32_t mediaId, const uint32_t startIndex, const uint32_t size) noexcept
	{
		uint32_t endIndex = startIndex + size;
		
		// Find how many images could fit in the cache
		if (size == 0)
		{
			uint32_t index = 0;
			uint64_t accumulatedByteSize = this->m_Cache->m_BytesSize;
			endIndex = 0;

			while (true)
			{
				const Image* tmpImage = this->GetImage(mediaId, index);

				if (tmpImage == nullptr) break;

				accumulatedByteSize += tmpImage->m_Stride;

				if (accumulatedByteSize > this->m_Cache->m_BytesCapacity) break;

				++endIndex;
				index = (index + 1) % static_cast<uint32_t>(this->m_Medias[mediaId]->GetRange().y);
			}

			endIndex += startIndex;
		}

		// if (startIndex != this->m_Range.x) --endIndex;
		for (uint32_t i = startIndex; i < endIndex; i++)
		{
			std::unique_lock<std::mutex> lock(this->m_Mutex);

			const uint32_t idx = i % static_cast<uint32_t>(this->m_Medias[mediaId]->GetRange().y);

			this->LoadImageToCache(mediaId, idx);

			lock.unlock();

			if (this->m_StopBgLoad) break;
		}
	}

	void Loader::ResizeCache(const uint64_t size, const bool sizeInMB) noexcept
	{
		uint64_t newSize = size;

		if (sizeInMB) newSize = Utils::MegaToBytes(size);

		this->m_Logger->Log(LogLevel_Debug, "[LOADER] : Resizing cache (%lld bytes)", newSize);
		
		this->m_Cache->Resize(newSize, false);
	}

	void Loader::BackgroundLoad() noexcept
	{
		while(true)
		{
			std::unique_lock<std::mutex> bgLoaderLock(this->m_Mutex);

			this->m_CondVar.wait(bgLoaderLock, [this] { return this->m_NeedBgLoad || this->m_StopBgLoad; });

			if (this->m_StopBgLoad) 
			{
				this->m_Logger->Log(LogLevel_Debug, "[LOADER] : Stopping background loader");
				this->m_IsWorking = false;
				this->m_StopBgLoad = false;
				break;
			}
			else if (this->m_NeedBgLoad)
			{
				bgLoaderLock.unlock();
				
				this->LoadSequenceToCache(0, this->m_BgLoadFrameIndex, this->m_BgLoadChunkSize);

				this->m_NeedBgLoad = false;
			}

			bgLoaderLock.unlock();
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

	void Loader::StopCacheLoader() noexcept
	{
		if (this->m_IsWorking)
		{
			std::unique_lock<std::mutex> stopLock(this->m_Mutex);

			this->m_StopBgLoad = true;

			stopLock.unlock();

			this->m_CondVar.notify_all();
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		this->JoinWorkers();
	}

	void Loader::JoinWorkers() noexcept
	{
		for (auto& worker : this->m_Workers)
		{
			if (worker.joinable()) worker.join();
		}
	}

	void Loader::Release() noexcept
	{
		// Stop cache loader if active
		this->StopCacheLoader();

		// Join left workers
		this->JoinWorkers();
		
		// Release cache
		if (this->m_Cache->m_HasBeenInitialized) this->m_Cache->Release();
		delete this->m_Cache;

		// Clear the medias
		for(auto& [id, media] : this->m_Medias) 
		{
			media->Release();
			delete media;
		}

		this->m_Medias.clear();

		// Clear workers
		this->m_Workers.clear();
		this->m_Workers.resize(0);

		this->m_Logger->Log(LogLevel_Diagnostic, "[LOADER] : Released loader");
	}
}