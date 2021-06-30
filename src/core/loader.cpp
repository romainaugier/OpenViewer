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
	
	const Imath::Box2i win = in.dataWindow();
	const Imath::V2i dim(win.max.x - win.min.x + 1, win.max.y - win.max.y + 1);

	const int dx = win.min.x;
	const int dy = win.min.y;

	in.setFrameBuffer((Imf::Rgba*)buffer - dx - dy * dim.x, 1, dim.x);
	in.readPixels(win.min.y, win.max.y);
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

void Image::Load(void* __restrict buffer) const noexcept
{
	if (type & FileType_Exr) LoadExr((half*)buffer);
	else if (type & FileType_Other) LoadOther((half*)buffer);
}

// initializes the loader with the different paths, the item count and the first frame
void Loader::Initialize(const std::string fp, const uint64_t _cache_size, bool isdirectory, Profiler& prof) noexcept
{
	has_been_initialized = 1;

	if (isdirectory)
	{
		cache_size = _cache_size;

		// allocate the cache
		if (_cache_size > 0 && use_cache > 0)
		{
			memory_arena = _aligned_malloc(cache_size, 16);
			use_cache = 1;
		}

		for (auto p : std::filesystem::directory_iterator(fp))
		{
			count++;
			const std::string t = p.path().u8string();
			images.emplace_back(t);
		}

		cached_size += images[0].size;

		// get the image file format to set the different buffers we need
		if (images[0].type & FileType_Exr || images[0].type & FileType_Other)
		{
			if (use_cache < 1) memory_arena = _aligned_malloc(images[0].size * sizeof(half), 16);
			cache_stride = cached_size;
			cached_size *= sizeof(half);
		}

		else if (images[0].type & FileType_Png)
		{
			if (use_cache < 1) memory_arena = _aligned_malloc(images[0].size * sizeof(uint8_t), 16);
			cache_stride = cached_size;
			cached_size *= sizeof(uint8_t);
		}

		// TODO : implement other file types

		// reserve the memory for the cache indices vector and set all to zero
		cached.resize(count);
		std::fill(cached.begin(), cached.end(), 0);

		// load the first frame to have something to show
		images[0].Load(memory_arena);
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

			if (images[0].type & FileType_Exr) memory_arena = _aligned_malloc(images[0].size * sizeof(half), 16);
			if (images[0].type & FileType_Png) memory_arena = _aligned_malloc(cached_size * sizeof(uint8_t), 16);
			if (images[0].type & FileType_Other) memory_arena = _aligned_malloc(cached_size * sizeof(half), 16);

			// TODO : implement other file types

			count = 1;
			images[0].Load(memory_arena);
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
}

// loads a single image
void Loader::LoadImage(const uint16_t idx, Profiler& prof) noexcept
{
	if (cached[idx] == 0)
	{
		images[idx].Load(memory_arena);
		cached_size += images[idx].size;
		cached[idx] = 1;
		last_cached.push_back(idx);

		UnloadImage();
	}
}

// unloads the first image in the cache
void Loader::UnloadImage() noexcept
{
	const uint16_t idx = last_cached[0];
	last_cached.erase(last_cached.begin());

	cached[idx] = 0;
	cached_size -= images[idx].size;
}

// loads a number of images in the cache
void Loader::LoadImages(const uint16_t idx, const uint8_t number) noexcept
{
	last_cached.reserve(number);

// #pragma omp parallel for
	for (int i = idx; i < (idx + number); i++)
	{
		const uint16_t index = i % (count - 1);
		//images[index].load(&memory_arena[(cache_stride * i) % cache_size_count]);
		cached_size += images[index].size;
		cached[index] = 1;
		last_cached.push_back(index);
	}

	has_finished = 1;
	is_working = 0;
}

// unloads a number of images from the cache
void Loader::UnloadImages(const uint8_t number) noexcept
{
	for (int i = 0; i < number; i++)
	{
		const uint16_t idx = last_cached[i];
		cached_size -= images[idx].size;
		cached[idx] = 0;
	}

	last_cached.erase(last_cached.begin(), last_cached.begin() + number);
}

// loads a sequence of images - used when OpenViewer is launched with an image sequence as an argument
void Loader::LoadSequence() noexcept
{
	uint64_t cached_amount = 0;

//#pragma omp parallel for
	for (int i = 1; i < cache_size_count; i++)
	{
		// mtx.lock();
//#pragma omp critical
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

			images[i].Load(address);
			images[i].cache_address = address;
			cached[i] = 1;
			last_cached.push_back(i);
			cached_amount += images[i].size;
		}
		// mtx.unlock();
	}

	cached_size += cached_amount;

	has_finished = 1;
}

// loads chunks of images when the viewer is playing
void Loader::LoadPlayer() noexcept
{
	while (true)
	{
		if (is_playing == 1 && cached[(frame + 5) % count] < 1)
		{
			LoadImages(frame + 5, 5);
			UnloadImages(5);
		}
		else if (stop_playloader > 0) break;

		else std::this_thread::sleep_for(std::chrono::milliseconds(16));
	}
}

// launch a worker thread to load an image sequence
void Loader::LaunchSequenceWorker() noexcept
{
	workers.emplace_back(&Loader::LoadSequence, this);

	has_finished = 0;
	is_working = 1;
}

// launches a worker thread that loads an certain amount of images in the cache
void Loader::LaunchCacheLoadWorker(const uint16_t idx, const uint16_t number) noexcept
{
	workers.emplace_back(&Loader::LoadImages, this, idx, number);

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
	
	for (auto& worker : workers) worker.join();
	
	// has_finished = 1;

	//if (is_playloader_working == 1) is_playloader_working = 0;
}

// release all the images and paths in case of reload
void Loader::Release() noexcept
{
	_aligned_free(memory_arena);
	memory_arena = nullptr;
	workers.clear();
	cached.clear();
	last_cached.clear();
	images.clear();

	count = 0;
}