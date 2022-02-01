// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 Romain Augier
// All rights reserved.

#include "timeline.h"

namespace Core
{
    void Timeline::Initialize(const ImVec2& range, Logger* logger, Loader* loader) noexcept
    {
        this->m_Loader = loader;
        this->m_Logger = logger;
        this->m_Range = range;

        this->m_Logger->Log(LogLevel_Diagnostic, "[TIMELINE] : Initializing timeline");

        this->m_CachedIndices.resize(range.y);
        for (uint32_t i = 0; i < range.y; i++) this->m_CachedIndices[i] = false;

        this->m_PlayerThread = std::thread(&Timeline::BackgroundTimeUpdate, this);
    }

    void Timeline::Add(Media* media) noexcept
    {
        Sequence* newSequence = new Sequence;
        newSequence->m_Media = media;
        newSequence->m_Range = media->GetRange();

        this->m_Sequences[++this->m_SequenceCount] = newSequence; 
    }

    void Timeline::Remove(const uint32_t mediaId) noexcept
    {
        this->m_Sequences.erase(mediaId);
    }

    void Timeline::Update() noexcept
	{
		if (this->m_Loader->m_HasBeenInitialized)
		{
			// Update the range of the loader to match the one of the playbar
			this->m_Loader->m_Range = this->m_Range;

			if (!this->m_Play)
			{
				if (this->m_Update)
				{
					// this->m_Loader->LoadImageToCache(this->m_MediaId, this->m_Frame);
				}
			}

			// Update cache indices
			for (uint32_t i = this->m_Range.x; i < this->m_Range.y; i++)
			{
				// const Core::Image* tmpImage = this->m_Loader->GetImage(this->m_MediaId, i);

				// this->m_CachedIndices[i] = tmpImage->m_CacheIndex > 0 ? 1 : 0;
			}
		}
	}

    Media* Timeline::GetMediaAtFrame(const uint32_t frame) noexcept
    {
		for (const auto& [id, sequence] : this->m_Sequences)
		{
			if (sequence->InRange(frame)) return sequence->m_Media;
		}

		return nullptr;
    }

    void Timeline::NeedUpdate(const bool need) noexcept
	{
		std::unique_lock<std::mutex> lock(this->m_Mutex);
		this->m_Update = need;
		lock.unlock();

		this->m_CV.notify_all();
	}

    void Timeline::Play() noexcept
    {
        std::unique_lock<std::mutex> playLock(this->m_Mutex);

		this->m_Play = true; 	
		this->m_Pause = false;

		playLock.unlock();

		this->m_CV.notify_all();
    }

    void Timeline::Pause() noexcept 
	{ 
		std::unique_lock<std::mutex> playLock(this->m_Mutex);

		this->m_Play = false; 
		this->m_Pause = true;	

		playLock.unlock();

		this->m_CV.notify_all();
	}

    void Timeline::GoPreviousFrame() noexcept
	{
		this->Pause();

		std::unique_lock<std::mutex> playLock(this->m_Mutex);

		this->m_Frame = this->m_Frame - 1 <= static_cast<uint32_t>(this->m_Range.x) || this->m_Frame == 0 ? 
						static_cast<uint32_t>(this->m_Range.y - 1) : 
						this->m_Frame - 1;
		
		playLock.unlock();
		
		this->NeedUpdate();	
	}

	void Timeline::GoNextFrame() noexcept
	{
		this->Pause();

		std::unique_lock<std::mutex> playLock(this->m_Mutex);

		this->m_Frame = this->m_Frame + 1 >= static_cast<uint32_t>(this->m_Range.y) ? 
						static_cast<uint32_t>(this->m_Range.x) : 
						this->m_Frame + 1; 

		playLock.unlock();

		this->NeedUpdate();
	}

    void Timeline::GoFirstFrame() noexcept
	{
		this->Pause();
	
		std::unique_lock<std::mutex> playLock(this->m_Mutex);

		this->m_Frame = this->m_Range.x;

		playLock.unlock();

		this->NeedUpdate();	
	}

	void Timeline::GoLastFrame() noexcept
	{
		this->Pause();
	
		std::unique_lock<std::mutex> playLock(this->m_Mutex);
	
		this->m_Frame = this->m_Range.y - 1;
		
		playLock.unlock();
		
		this->NeedUpdate();	
	}

    void Timeline::BackgroundTimeUpdate() noexcept
	{
		uint32_t sleepTime = static_cast<uint32_t>(1.0f / static_cast<float>(this->m_FrameRate) * 1000.0f);
		
		while (true)
		{
			std::unique_lock<std::mutex> bgUpdateLock(this->m_Mutex);

			this->m_CV.wait_for(bgUpdateLock, std::chrono::milliseconds(sleepTime), [this] { return this->m_Release || this->m_Pause; });

			sleepTime = static_cast<uint32_t>(1.0f / static_cast<float>(this->m_FrameRate) * 1000.0f);

			if (this->m_Release)
			{
				bgUpdateLock.unlock();
				break;
			}

			if (this->m_Pause)
			{
				bgUpdateLock.unlock();
			}
			else if (this->m_Play)
			{
				const auto updateStart = Profiler::StaticStart();

				this->m_Frame = fmodf(this->m_Frame + 1, (this->m_Range.y));
				this->m_Update = true;

				// Notify the background cache loader that we need to store some frames in the cache
				if (this->m_Loader->m_CacheMode > 0)
				{
					// Urgent load in case of fast scrolling while cache is activated
					// if (!this->m_CachedIndices[this->m_Frame]) this->m_Loader->LoadImageToCache(this->m_MediaId, this->m_Frame);

					const uint32_t offset = this->m_Loader->m_Cache->m_Size - this->m_Loader->m_BgLoadChunkSize * 2;
					const uint32_t frameIdx = static_cast<uint32_t>(fmodf((this->m_Frame + offset), (this->m_Range.y - 1)));

					if (!this->m_CachedIndices[frameIdx])
					{
						std::unique_lock<std::mutex> playbarLock(this->m_Loader->m_Mutex);

						this->m_Loader->m_NeedBgLoad = true;
						this->m_Loader->m_BgLoadFrameIndex = frameIdx;

						playbarLock.unlock();
					}
				}
				else
				{
					// this->m_Loader->LoadImageToCache(this->m_MediaId, this->m_Frame);
				}

				const auto updateEnd = Profiler::StaticEnd();

				const float loadTime = Profiler::StaticTime(updateStart, updateEnd);

				this->m_RealFramerate = 1000.0f / std::max(loadTime, static_cast<float>(sleepTime));

				sleepTime = sleepTime - std::min(static_cast<uint32_t>(loadTime), sleepTime) + 1;
				
				bgUpdateLock.unlock();
			}
		}
	}

    void Timeline::Release() noexcept
    {
        std::unique_lock<std::mutex> lock(this->m_Mutex);
		this->m_Release = true;
		lock.unlock();

		// Make sure the thread has finished working
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

		// Join it
		if (this->m_PlayerThread.joinable()) this->m_PlayerThread.join();
        
        for (auto it = this->m_Sequences.cbegin(); it != this->m_Sequences.cend(); ++it)
        {
            delete it.value();
        }

        this->m_Logger->Log(LogLevel_Diagnostic, "[TIMELINE] : Released timeline");
    }
}