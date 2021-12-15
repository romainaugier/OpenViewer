// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#include "loader.h"

// initializes the loader with the different paths, the item count and the first frame
void Loader::Initialize(const std::string fp, const uint64_t _cache_size, bool isdirectory) noexcept
{
	this->logger->Log(LogLevel_Debug, "[LOADER] : Initializing Loader with %s", fp.c_str());
	
	this->has_been_initialized = 1;

	if (isdirectory)
	{
		this->cache_size = _cache_size;

		// TODO : make a real system to detect different image sequences in the same directory
		// and let the user choose them
		for (auto p : std::filesystem::directory_iterator(fp))
		{
			this->count++;
			const std::string t = p.path().u8string();
			this->images.emplace_back(t);
			this->logger->Log(LogLevel_Debug, "Loading : %s", t.c_str());
		}

		// allocate the cache
		if (use_cache > 0)
		{
			this->logger->Log(LogLevel_Debug, "[LOADER] : Loader cache size is %lld bytes", static_cast<long long>(_cache_size));
			
			// make sure memory is not already allocated
			if (this->memory_arena != nullptr) OvFree(this->memory_arena);

			this->memory_arena = OvAlloc(this->cache_size, 16);
			this->use_cache = 1;
		}
		else
		{
			uint8_t img_type_size, img_format_size;
			this->images[0].GetTypeSize(img_format_size, img_type_size);

			if (this->memory_arena != nullptr) OvFree(this->memory_arena);

			this->cache_size = this->images[0].size * img_type_size;
			this->memory_arena = OvAlloc(this->cache_size, 16);
			this->use_cache = 0;
		}

		this->logger->Log(LogLevel_Debug, "[LOADER] : Image size is %d bytes.", this->images[0].size * sizeof(half));

		this->cached_size = this->images[0].size * sizeof(half);
		this->cache_stride = this->images[0].size;

		// get the image file format to set the different buffers we need
		//if (images[0].type & FileType_Exr || images[0].type & FileType_Other)
		//{
		//	// make sure memory is not already allocated
		//
		//}

		// TODO : implement other file types

		// reserve the memory for the cache indices vector and set all to zero
		cached.resize(count);
		std::fill(cached.begin(), cached.end(), 0);

		// load the first frame to have something to show
		images[0].Load(memory_arena, profiler);
		images[0].cache_address = memory_arena;
		cached[0] = 1;
		cache_size_count = round(cache_size / cached_size);
		cache_size_count = cache_size_count > count ? count : cache_size_count;
		last_cached.push_back(0);

		last_cached.reserve(cache_size_count);
	}
	else
	{
		if (std::filesystem::exists(fp))
		{
			images.emplace_back(fp);

			cached_size += images[0].size;

			if (images[0].type & FileType_Exr) memory_arena = OvAlloc(images[0].size * sizeof(half), 16);
			else if (images[0].type & FileType_Other) memory_arena = OvAlloc(cached_size * sizeof(half), 16);

			// TODO : implement other file types

			count = 1;
			images[0].Load(memory_arena, profiler);
			images[0].cache_address = memory_arena;
			cached.resize(1);
			cached[0] = 1;
			last_cached.push_back(0);
		}
		else
		{
			// TODO : handle file that does not exist
		}
	}

	logger->Log(LogLevel_Debug, "[LOADER] : Cache can handle %d image(s).", cache_size_count);
}

// reallocates the image cache. if we use a full cache, allocates the full cache like in the initializer,
// else allocate enough memory to store one image
void Loader::ReallocateCache(const bool use_cache) noexcept
{
	if (use_cache)
	{
		memory_arena = OvAlloc(cache_size, 16);

		logger->Log(LogLevel_Debug, "[LOADER] : Cache reallocation size : %lld", cache_size);

		cache_size_count = round(cache_size / (images[0].size * sizeof(half)));
		cache_size_count = cache_size_count > count ? count : cache_size_count;

		last_cached.reserve(cache_size_count);

		logger->Log(LogLevel_Debug, "[LOADER] : Cache reallocation : cache can now handle %d image(s)", cache_size_count);
	}
	else
	{
		memory_arena = OvAlloc(images[0].size * sizeof(half), 16);
	}
}

// loads a single image
void Loader::LoadImage(const uint16_t idx, void* address) noexcept
{
	if (cached[idx] == 0)
	{
		if (use_cache < 1) address = memory_arena;

		images[idx].Load(address, profiler);
		cached_size += (images[idx].size * sizeof(half));
		cached[idx] = 1;

		if (last_cached.size() < 1) last_cached.push_back(idx);
		else last_cached[last_cached.size() - 1] = idx;
	}
}

// nloads the first image in the cache and returns its address
void* Loader::UnloadImage() noexcept
{
	if (last_cached.size() > 0)
	{
		const uint16_t idx = last_cached[0];
		cached[idx] = 0;

		if (last_cached.size() > 1) memmove(&last_cached[0], &last_cached[1], (last_cached.size() - 1) * sizeof(uint16_t));

		void* tmp = images[idx].cache_address;
		images[idx].cache_address = nullptr;
		cached_size -= (images[idx].size * sizeof(half));

		if (tmp == nullptr)
		{
			logger->Log(LogLevel_Debug, "[LOADER] : Problem with memory, using source pointer.");
			tmp = memory_arena;
		}

		return tmp;
	}
	else
	{
		logger->Log(LogLevel_Debug, "[LOADER] : Unload Image returned nullptr (%d).", last_cached.size());
		return nullptr;
	}
}

// loads a sequence of images - used when OpenViewer is launched with an image sequence as an argument
void Loader::LoadSequence(const bool load_first_frame) noexcept
{
	uint64_t cached_amount = 0;

	logger->Log(LogLevel_Debug, "[LOADER] : Starting sequence loading.");

	for (int i = 0; i < cache_size_count; i++)
	{
		{
			if (!load_first_frame && i == 0) i = 1;
			
			void* address = nullptr;

			auto tmp_address = static_cast<half*>(memory_arena);
			address = static_cast<void*>(&tmp_address[cache_stride * i]);

			images[i].Load(address, profiler);
			images[i].cache_address = address;
			cached[i] = 1;
			last_cached.push_back(i);
			cached_amount += (images[i].size * sizeof(half));
		}
	}

	cached_size += cached_amount;

	logger->Log(LogLevel_Debug, "[LOADER] : Finished sequence loading.");

	has_finished = 1;
}

// loads chunks of images when the viewer is playing
void Loader::LoadPlayer() noexcept
{
	while (true)
	{
		std::unique_lock<std::mutex> lock(mtx);

		load_into_cache.wait(lock, [this] { return work_for_cache || stop_playloader; });	

		if (stop_playloader > 0)
		{
			logger->Log(LogLevel_Debug, "[LOADER] : Load player thread exiting");
			is_playloader_working = 0;
			break;
		}
		else if (work_for_cache > 0)
		{
			uint16_t index = urgent_load > 0 ? 0 : cache_size_count / 2;
			
			for (uint8_t i = 0; i < (cache_size_count / 4); i++)
			{
				if (cached[(cache_load_frame + index + i) % count] == 0)
				{
					void* address = UnloadImage();

					if (address == nullptr)
					{
						address = memory_arena;
					}

					LoadImage((cache_load_frame + index + i) % (count), address);
				}
				else continue;
			}

			work_for_cache = 0;
		}
		
		lock.unlock();
		load_into_cache.notify_all();
	}
}

// launch a worker thread to load an image sequence
void Loader::LaunchSequenceWorker(const bool load_first_frame) noexcept
{
	workers.emplace_back(&Loader::LoadSequence, this, load_first_frame);

	has_finished = 0;
	is_working = 1;
}

// launches the player loading worker
void Loader::LaunchPlayerWorker() noexcept
{
	workers.emplace_back(&Loader::LoadPlayer, this);

	has_finished = 0;
	is_working = 1;
	is_playloader_working = 1;
}

// join the last worker
void Loader::JoinWorker() noexcept
{
	for (auto& worker : workers)
	{
		if(worker.joinable()) worker.join();
	}
}

// releases the image cache
void Loader::ReleaseCache() noexcept
{
	OvFree(memory_arena);
	memory_arena = nullptr;
	memset(&cached[0], 0, cached.size() * sizeof(cached[0]));
	last_cached.clear();
	last_cached.resize(0);
	cached_size = 0;
}

// release all the images and paths in case of reload
void Loader::Release() noexcept
{
	ReleaseCache();

	workers.clear();
	workers.resize(0);
	cached.clear();
	cached.resize(0);
	last_cached.clear();
	last_cached.resize(0);
	images.clear();
	images.resize(0);

	count = 0;
	cached_size = 0;
}