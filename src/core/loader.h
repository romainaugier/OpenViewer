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
#include <malloc.h>

#include "utils/string_utils.h"
#include "utils/profiler.h"
#include "utils/logger.h"
#include "utils/memory/alloc.h"

#undef LoadImage

enum FileType_
{
	FileType_Exr   = 0x1,
	FileType_Png   = 0x2,
	FileType_Jpg   = 0x4,
	FileType_Mov   = 0x8,
	FileType_Mp4   = 0x10,
	FileType_Other = 0x20
};

enum Format_
{
	Format_RGBA_FLOAT = 0x1,
	Format_RGB_FLOAT  = 0x2,
	Format_RGBA_U8    = 0x4,
	Format_RGB_U8     = 0x8,
	Format_RGBA_U32   = 0x10,
	Format_RGB_U32    = 0x20,
	Format_RGBA_HALF  = 0x40,
	Format_RGB_HALF   = 0x80
};

struct Image
{
	std::string path;
	uint64_t size;
	void* cache_address = nullptr;
	uint32_t xres;
	uint32_t yres;
	uint32_t channels;
	int16_t cache_index = -1;
	FileType_ type;
	Format_ format;
	GLint internal_format;
	GLenum gl_format;
	GLenum gl_type;

	Image() {}

	Image(const std::string& fp)
	{
		path = fp;
		
		auto in = OIIO::ImageInput::open(fp);
		const OIIO::ImageSpec& spec = in->spec();

		if(endsWith(fp, ".exr"))
		{
			type = FileType_Exr;
			format = Format_RGBA_HALF;
			internal_format = GL_RGBA32F;
			gl_format = GL_RGBA;
			gl_type = GL_FLOAT;
			xres = spec.full_width;
			yres = spec.full_height;
			channels = spec.nchannels + 1;
		}
		else if(endsWith(fp, ".png"))
		{
			type = FileType_Other;
			format = Format_RGB_U8;
			internal_format = GL_RGBA32F;
			gl_format = GL_RGBA;
			gl_type = GL_FLOAT;
			xres = spec.width;
			yres = spec.height;
			channels = spec.nchannels;
		}
		else if(endsWith(fp, ".jpg") || endsWith(fp, ".jpeg"))
		{
			type = FileType_Other;
			format = Format_RGB_U8;
			internal_format = GL_RGBA32F;
			gl_format = GL_RGBA;
			gl_type = GL_FLOAT;
			xres = spec.width;
			yres = spec.height;
			channels = spec.nchannels;
		}
		else
		{
			type = FileType_Other;
			format = Format_RGB_HALF;
			internal_format = GL_RGBA32F;
			gl_format = GL_RGBA;
			gl_type = GL_FLOAT;
			xres = spec.width;
			yres = spec.height;
			channels = spec.nchannels;
		}


		size = static_cast<uint64_t>(xres) * yres * channels; // *sizeof(float);

		in->close();
	}

	void Release() noexcept;
	void Load(void* __restrict buffer, Profiler* prof) noexcept;
	void LoadExr(half* __restrict buffer) const noexcept;
	void LoadPng(uint8_t* __restrict buffer) const noexcept;
	void LoadJpg(uint8_t* __restrict buffer) const noexcept;
	void LoadOther(half* __restrict allocated_space) const noexcept;
};

struct Loader
{
	std::vector<Image> images;
	std::vector<std::thread> workers;
	std::vector<uint16_t> last_cached;
	std::vector<char> cached;
	std::mutex mtx;
	std::condition_variable load_into_cache;
	void* memory_arena = nullptr;
	Profiler* profiler;
	Logger* logger;
	uint64_t cache_size;
	uint64_t cached_size;
	uint64_t cache_stride;
	uint16_t count = 0;
	uint16_t frame = 0;
	uint16_t cache_size_count = 0;
	uint16_t cache_load_frame = 0;
	unsigned int urgent_load : 1;
	unsigned int work_for_cache : 1;
	unsigned int has_been_initialized : 1;
	unsigned int use_cache : 1;
	unsigned int is_playing : 1;
	unsigned int has_finished : 1;
	unsigned int is_working : 1;
	unsigned int is_playloader_working : 1;
	unsigned int stop_playloader : 1;

	Loader(Profiler* prof, Logger* log) 
	{
		has_been_initialized = 0;
		use_cache = 0;
		cached_size = 0;
		has_finished = 0;
		is_working = 0;
		is_playloader_working = 0;
		stop_playloader = 0;
		work_for_cache = 0;
		urgent_load = 0;
		profiler = prof;
		logger = log;
	}

	~Loader()
	{
	}

	void Initialize(const std::string fp, const uint64_t _cache_size, bool isdirectory) noexcept;
	void ReallocateCache(const bool use_cache) noexcept;
	void LoadSequence(const bool load_first_frame) noexcept;
	void LoadPlayer() noexcept;
	void LoadImage(const uint16_t idx, void* address) noexcept;
	void* UnloadImage() noexcept;
	void LaunchSequenceWorker(const bool load_first_frame) noexcept;
	void LaunchPlayerWorker() noexcept;
	void JoinWorker() noexcept;
	void ReleaseCache() noexcept;
	void Release() noexcept;
};

