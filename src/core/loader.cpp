// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#include "loader.h"

// releases an image data
void Image::Release() noexcept
{
	cache_index = -1;
	cache_address = nullptr;
}

// loads an image
void Image::LoadExr(half* __restrict buffer) const noexcept
{
	Imf::RgbaInputFile in(path.c_str());
	
	const Imath::Box2i display = in.displayWindow();
	const Imath::Box2i data = in.dataWindow();
	const Imath::V2i dim(data.max.x - data.min.x + 1, data.max.y - data.max.y + 1);

	const int dx = data.min.x;
	const int dy = data.min.y;
	
	// in case the data window is smaller than the display window
	// we fill empty pixels everywhere and then read the image pixels
	// to avoid non initialized values in memory
	if (data.min.x > display.min.x || data.max.x < display.max.x || 
		data.min.y > display.min.y || data.max.y < display.min.y)
	{
		memset(&buffer[0], 0.0f, size);
	}

	in.setFrameBuffer((Imf::Rgba*)buffer, 1, dim.x);
	in.readPixels(data.min.y, data.max.y);
}

void Image::LoadPng(uint8_t* __restrict buffer) const noexcept
{

}

void Image::LoadJpg(uint8_t* __restrict buffer) const noexcept
{

}

void Image::LoadOther(half* __restrict buffer) const noexcept
{
	auto in = OIIO::ImageInput::open(path);
	in->read_image(0, -1, OIIO::TypeDesc::HALF, (half*)buffer);
	in->close();
}

void Image::Load(void* __restrict buffer, Profiler* prof) noexcept
{
	cache_address = buffer;
	
	auto load_timer_start = prof->Start();

	if (type & FileType_Exr) LoadExr((half*)buffer);
	else if (type & FileType_Other) LoadOther((half*)buffer);

	auto load_timer_end = prof->End();
	prof->Load(load_timer_start, load_timer_end);
}

// initializes the loader with the different paths, the item count and the first frame
void Loader::Initialize(const std::string fp, const uint64_t _cache_size, bool isdirectory) noexcept
{
	logger->Log(LogLevel_Debug, "Initializing Loader with %s", fp.c_str());
	
	has_been_initialized = 1;

	if (isdirectory)
	{
		cache_size = _cache_size;

		logger->Log(LogLevel_Debug, "Loader cache size is %lld bytes", static_cast<long long>(_cache_size));

		for (auto p : std::filesystem::directory_iterator(fp))
		{
			count++;
			const std::string t = p.path().u8string();
			images.emplace_back(t);
			// logger->Log(LogLevel_Debug, "Loading : %s", t.c_str());
		}

		// allocate the cache
		if (use_cache > 0)
		{
			// make sure memory is not already allocated
			if (memory_arena != nullptr) OvFree(memory_arena);

			memory_arena = OvAlloc(cache_size * 2, 16);
			use_cache = 1;
		}
		else
		{
			if (memory_arena != nullptr) OvFree(memory_arena);

			memory_arena = OvAlloc(images[0].size * sizeof(half), 16);
			cache_size = images[0].size * sizeof(half);
			//cached_size *= sizeof(half);
		}

		logger->Log(LogLevel_Debug, "Image size is %d bytes.", images[0].size * sizeof(half));
		logger->Log(LogLevel_Debug, "Size of half is %d", sizeof(half));

		cached_size = images[0].size * sizeof(half);
		cache_stride = cached_size;

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

	logger->Log(LogLevel_Debug, "Cache can handle %d image(s).", cache_size_count);
}

// reallocates the image cache. if we use a full cache, allocates the full cache like in the initializer,
// else allocate enough memory to store one image
void Loader::ReallocateCache(const bool& use_cache) noexcept
{
	if (use_cache)
	{
		memory_arena = OvAlloc(cache_size * 2, 16);
	}
	else
	{
		memory_arena = OvAlloc(images[0].size * sizeof(half), 16);
	}
}

// loads a single image
void Loader::LoadImage(const uint16_t idx, void* address) noexcept
{
	if (use_cache < 1) address = memory_arena;
	
	images[idx].Load(address, profiler);
	cached_size += (images[idx].size * sizeof(half));
	cached[idx] = 1;
	last_cached[last_cached.size() - 1] = idx;
}

// unloads the first image in the cache and returns its address
void* Loader::UnloadImage() noexcept
{
	const uint16_t idx = last_cached[0];
	if(last_cached.size() > 1) memmove(&last_cached[0], &last_cached[1], (last_cached.size() - 1) * sizeof(uint16_t));

	cached[idx] = 0;
	void* tmp = images[idx].cache_address;
	images[idx].cache_address = nullptr;
	cached_size -= (images[idx].size * sizeof(half));

	return tmp;
}

// loads a sequence of images - used when OpenViewer is launched with an image sequence as an argument
void Loader::LoadSequence() noexcept
{
	uint64_t cached_amount = 0;

	for (int i = 1; i < cache_size_count; i++)
	{
		{
			void* address = nullptr;

			if (images[i].type & FileType_Exr || images[i].type & FileType_Other)
			{
				auto tmp_address = static_cast<half*>(memory_arena);
				address = (void*)&tmp_address[cache_stride * i];
			}
			else if (images[i].type & FileType_Png)
			{
				auto tmp_address = static_cast<uint8_t*>(memory_arena);
				address = (void*)&tmp_address[cache_stride * i];
			}

			images[i].Load(address, profiler);
			images[i].cache_address = address;
			cached[i] = 1;
			last_cached.push_back(i);
			cached_amount += (images[i].size * sizeof(half));
		}
	}

	cached_size += cached_amount;

	has_finished = 1;
}

// loads chunks of images when the viewer is playing
void Loader::LoadPlayer() noexcept
{
	while (true)
	{
		std::unique_lock<std::mutex> lock(mtx);
		load_into_cache.wait(lock, [this] { return work_for_cache; });		
		if (work_for_cache > 0)
		{
			uint16_t index = urgent_load > 0 ? 0 : cache_size_count / 2;
			
			for (uint8_t i = 0; i < (cache_size_count / 4); i++)
			{
				if (cached[(cache_load_frame + index + i) % count] == 0)
				{
					void* address = UnloadImage();
					LoadImage((cache_load_frame + index + i) % (count), address);
				}
				else continue;
			}

			work_for_cache = false;
		}
		else if (stop_playloader > 0)
		{
			logger->Log(LogLevel_Debug, "Load player thread exiting");
			break;
		}
		
		lock.unlock();
		load_into_cache.notify_all();
	}
}

// launch a worker thread to load an image sequence
void Loader::LaunchSequenceWorker() noexcept
{
	workers.emplace_back(&Loader::LoadSequence, this);

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
	//uint8_t idx = workers.size() - 1;
	//workers[idx].join();
	
	for (auto& worker : workers)
	{
		if(worker.joinable()) worker.join();
	}
	// has_finished = 1;

	//if (is_playloader_working == 1) is_playloader_working = 0;
}

// releases the image cache
void Loader::ReleaseCache() noexcept
{
	OvFree(memory_arena);
	memory_arena = nullptr;
	memset(&cached[0], 0, cached.size() * sizeof(cached[0]));
	last_cached.clear();
	last_cached.resize(0);
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