// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#pragma once

#include "OpenImageIO/imagecache.h"
#include "GL/gl3w.h"

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
	void* cache_address = nullptr;
	uint64_t size;
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

		if(endsWith(fp, ".exr"))
		{
			type = FileType_Exr;
			format = Format_RGBA_HALF;
			internal_format = GL_RGBA16F;
			gl_format = GL_RGBA;
			gl_type = GL_HALF_FLOAT;
		}
		else if(endsWith(fp, ".png"))
		{
			type = FileType_Png;
			format = Format_RGB_U8;
		}
		else if(endsWith(fp, ".jpg") || endsWith(fp, ".jpeg"))
		{
			type = FileType_Jpg;
			format = Format_RGB_U8;
		}
		else
		{
			type = FileType_Other;
			format = Format_RGB_HALF;
			internal_format = GL_RGB16F;
			gl_format = GL_RGB;
			gl_type = GL_HALF_FLOAT;
		}

		auto in = OIIO::ImageInput::open(fp);
		const OIIO::ImageSpec& spec = in->spec();

		xres = spec.width;
		yres = spec.height;
		channels = spec.nchannels == 3 ? spec.nchannels + 1 : spec.nchannels;

		size = xres * yres * channels; // *sizeof(float);

		in->close();
	}

	void Release() noexcept;
	void Load(void* __restrict buffer) const noexcept;
	void LoadExr(half* __restrict buffer) const noexcept;
	void LoadPng(uint8_t* __restrict buffer) const noexcept;
	void LoadJpg(uint8_t* __restrict buffer) const noexcept;
	void LoadOther(half* __restrict allocated_space) const noexcept;
};

struct Loader
{
	void* memory_arena = nullptr;
	std::vector<Image> images;
	std::vector<std::thread> workers;
	std::vector<uint16_t> last_cached;
	std::vector<char> cached;
	mutable std::mutex mtx;
	uint64_t cache_size;
	uint64_t cached_size;
	uint64_t cache_stride;
	uint16_t count = 0;
	uint16_t frame = 0;
	uint16_t cache_size_count = 0;
	unsigned int has_been_initialized : 1;
	unsigned int use_cache : 1;
	unsigned int is_playing : 1;
	unsigned int has_finished : 1;
	unsigned int is_working : 1;
	unsigned int is_playloader_working : 1;
	unsigned int stop_playloader : 1;

	Loader() 
	{
		has_been_initialized = 0;
		use_cache = 1;
		cached_size = 0;
		has_finished = 0;
		is_working = 0;
		is_playloader_working = 0;
		stop_playloader = 0;
	}

	~Loader()
	{
	}

	void Initialize(const std::string fp, const uint64_t _cache_size, bool isdirectory, Profiler& prof) noexcept;
	void LoadSequence() noexcept;
	void LoadPlayer() noexcept;
	void LoadImages(const uint16_t idx, const uint8_t number) noexcept;
	void LoadImage(const uint16_t idx, Profiler& prof) noexcept;
	void UnloadImages(const uint8_t number) noexcept;
	void UnloadImage() noexcept;
	void LaunchSequenceWorker() noexcept;
	void LaunchCacheLoadWorker(const uint16_t idx, const uint16_t number) noexcept;
	void LaunchPlayerWorker() noexcept;
	void JoinWorker() noexcept;
	void Release() noexcept;
};

