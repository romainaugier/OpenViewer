// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#include "loader.h"

// releases an image data
void Image::release()
{
	cache_index = -1;
}

// loads an image
void Image::load_exr(half* __restrict buffer) noexcept
{
	Imf::RgbaInputFile in(path.c_str());
	
	Imath::Box2i win = in.dataWindow();
	Imath::V2i dim(win.max.x - win.min.x + 1, win.max.y - win.max.y + 1);

	int dx = win.min.x;
	int dy = win.min.y;

	in.setFrameBuffer((Imf::Rgba*)buffer - dx - dy * dim.x, 1, dim.x);
	in.readPixels(win.min.y, win.max.y);
}

void Image::load_png(uint8_t* __restrict buffer) noexcept
{

}

void Image::load_jpg(uint8_t* __restrict buffer) noexcept
{

}

void Image::load_other(half* __restrict buffer) noexcept
{
	auto in = OIIO::ImageInput::open(path);
	in->read_image(0, -1, OIIO::TypeDesc::HALF, (half*)buffer);
	in->close();
}

void Image::load(void* __restrict buffer) noexcept
{
	if (type & FileType_Exr) load_exr((half*)buffer);
	else if (type & FileType_Other) load_other((half*)buffer);
}

// initializes the loader with the different paths, the item count and the first frame
void Loader::initialize(std::string fp, uint64_t _cache_size, bool isdirectory)
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
			std::string t = p.path().u8string();
			images.emplace_back(t);
		}

		cached_size += images[0].size;

		// get the image file format to set the different buffers we need
		if (images[0].type & FileType_Exr)
		{
			if (use_cache < 1) memory_arena = _aligned_malloc(images[0].size * sizeof(half), 16);
			cache_stride = cached_size * sizeof(half);
		}

		else if (images[0].type & FileType_Png)
		{
			if (use_cache < 1) memory_arena = _aligned_malloc(images[0].size * sizeof(uint8_t), 16);
			cache_stride = cached_size * sizeof(uint8_t);
		}

		// TODO : implement other file types

		// reserve the memory for the cache indices vector and set all to zero
		cached.resize(count);
		std::fill(cached.begin(), cached.end(), 0);

		// load the first frame to have something to show
		images[0].load(memory_arena);
		cached[0] = 1;
		cache_size_count = round(cache_size / cached_size);
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
			images[0].load(memory_arena);
			cached.resize(1);
			cached[0] = 1;
			last_cached.push_back(0);
		}
		else
		{
			// TODO : handle file does not exist
		}
	}
}

// loads a single image
void Loader::load_image(uint16_t idx)
{
	if (cached[idx] == 0)
	{
		images[idx].load(memory_arena);
		cached_size += images[idx].size;
		cached[idx] = 1;
		last_cached.push_back(idx);

		unload_image();
	}
}

// unloads the first image in the cache
void Loader::unload_image()
{
	uint16_t idx = last_cached[0];
	last_cached.erase(last_cached.begin());

	cached[idx] = 0;
	cached_size -= images[idx].size;
}

// loads a number of images in the cache
void Loader::load_images(uint16_t idx, uint8_t number)
{
	last_cached.reserve(number);

// #pragma omp parallel for
	for (int i = idx; i < (idx + number); i++)
	{
		uint16_t index = i % (count - 1);
		//images[index].load(&memory_arena[(cache_stride * i) % cache_size_count]);
		cached_size += images[index].size;
		cached[index] = 1;
		last_cached.push_back(index);
	}

	has_finished = 1;
	is_working = 0;
}

// unloads a number of images from the cache
void Loader::unload_images(uint8_t number)
{
	for (int i = 0; i < number; i++)
	{
		uint16_t idx = last_cached[i];
		cached_size -= images[idx].size;
		cached[idx] = 0;
	}

	last_cached.erase(last_cached.begin(), last_cached.begin() + number);
}

// loads a sequence of images - used when OpenViewer is launched with an image sequence as an argument
void Loader::load_sequence()
{
	// we wait a bit to make sure the ui is fully loaded
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	uint64_t cached_amount = 0;

//#pragma omp parallel for
	for (int i = 1; i < (cache_size_count - 1); i++)
	{
		// mtx.lock();
//#pragma omp critical
		{
			//images[i].load(&memory_arena[(cache_stride * i)]);
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
void Loader::load_player()
{
	while (true)
	{
		if (is_playing == 1 && cached[frame + 5] < 1)
		{
			load_images(frame + 5, 5);
			unload_images(5);
		}

		else std::this_thread::sleep_for(std::chrono::milliseconds(16));
	}
}

// launch a worker thread to load an image sequence
void Loader::launch_sequence_worker()
{
	workers.emplace_back(&Loader::load_sequence, this);

	has_finished = 0;
	is_working = 1;
}

// launches a worker thread that loads an certain amount of images in the cache
void Loader::launch_cacheload_worker(uint16_t idx, uint16_t number)
{
	workers.emplace_back(&Loader::load_images, this, idx, number);

	has_finished = 0;
	is_working = 1;
}

// launches the player loading worker
void Loader::launch_player_worker()
{
	workers.emplace_back(&Loader::load_player, this);

	has_finished = 0;
	is_working = 1;
	is_playloader_working = 1;
}

// join the last worker
void Loader::join_worker()
{
	uint8_t idx = workers.size() - 1;
	workers[idx].join();
	// has_finished = 1;

	//if (is_playloader_working == 1) is_playloader_working = 0;
}

// release all the images and paths in case of reload
void Loader::release()
{
	_aligned_free(memory_arena);
	memory_arena = nullptr;
	workers.clear();
	cached.clear();
	last_cached.clear();
	images.clear();

	count = 0;
}