// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include "stdint.h"

#include "GL/glew.h"
#include "OpenImageIO/imagebuf.h"
#include "OpenEXR/ImfRgbaFile.h"

#include "utils/string_utils.h"
#include "utils/logger.h"
#include "utils/profiler.h"

enum FileType_
{
	FileType_Exr   = 0x1,
	FileType_Png   = 0x2,
	FileType_Jpg   = 0x4,
	FileType_Tiff  = 0x8,
	FileType_Hdr   = 0X10,
	FileType_Mov   = 0x20,
	FileType_Mp4   = 0x40,
	FileType_Other = 0x80
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

namespace Core
{
	struct Image
	{
		std::string m_Path;

		uint64_t m_Size = 0;   // Size 
		uint64_t m_Stride = 0; // Size in bytes

		uint32_t m_Xres = 0;
		uint32_t m_Yres = 0;
		uint32_t m_Channels = 0;
		
		uint16_t m_CacheIndex = 0; // zero means it is not cached
		
		FileType_ m_Type;
		Format_ m_Format;
		GLint m_GLInternalFormat;
		GLenum m_GLFormat;
		GLenum m_GLType;

		Image() {}

		Image(const std::string& fp)
		{
			m_Path = fp;
			
			auto in = OIIO::ImageInput::open(fp);
			const OIIO::ImageSpec& spec = in->spec();

			if(Utils::EndsWith(fp, ".exr"))
			{
				m_Type = FileType_Exr;
				m_Format = Format_RGBA_HALF;
				m_GLInternalFormat = GL_RGBA32F;
				m_GLFormat = GL_RGBA;
				m_GLType = GL_HALF_FLOAT;
				m_Xres = spec.full_width;
				m_Yres = spec.full_height;
				m_Channels = spec.nchannels + 1;
				m_Size = m_Xres * m_Yres * m_Channels;
				m_Stride = m_Size * Size::Size16;
			}
			else if(Utils::EndsWith(fp, ".png"))
			{
				m_Type = FileType_Other;
				m_Format = Format_RGB_U8;
				m_GLInternalFormat = GL_RGBA32F;
				m_GLFormat = GL_RGB;
				m_GLType = GL_UNSIGNED_BYTE;
				m_Xres = spec.width;
				m_Yres = spec.height;
				m_Channels = spec.nchannels;
				m_Size = m_Xres * m_Yres * m_Channels;
				m_Stride = m_Size * Size::Size8;
			}
			else if(Utils::EndsWith(fp, ".jpg") || Utils::EndsWith(fp, ".jpeg"))
			{
				m_Type = FileType_Other;
				m_Format = Format_RGB_U8;
				m_GLInternalFormat = GL_RGB32F;
				m_GLFormat = GL_RGB;
				m_GLType = GL_UNSIGNED_BYTE;
				m_Xres = spec.width;
				m_Yres = spec.height;
				m_Channels = spec.nchannels;
				m_Size = m_Xres * m_Yres * m_Channels;
				m_Stride = m_Size * Size::Size8;
			}
			else if(Utils::EndsWith(fp, ".hdr"))
			{
				m_Type = FileType_Hdr;
				m_Format = Format_RGB_FLOAT;
				m_GLInternalFormat = GL_RGB32F;
				m_GLFormat = GL_RGB;
				m_GLType = GL_FLOAT;
				m_Xres = spec.width;
				m_Yres = spec.height;
				m_Channels = spec.nchannels;
				m_Size = m_Xres * m_Yres * m_Channels;
				m_Stride = m_Size * Size::Size32;
			}
			else if(Utils::EndsWith(fp, ".tiff"))
			{
				m_Type = FileType_Tiff;
				m_Format = Format_RGB_HALF;
				m_GLInternalFormat = GL_RGBA32F;
				m_GLFormat = GL_RGBA;
				m_GLType = GL_HALF_FLOAT;
				m_Xres = spec.width;
				m_Yres = spec.height;
				m_Channels = spec.nchannels;
				m_Size = m_Xres * m_Yres * m_Channels;
				m_Stride = m_Size * Size::Size16;
			}
			else
			{
				m_Type = FileType_Other;
				m_Format = Format_RGB_HALF;
				m_GLInternalFormat = GL_RGB32F;
				m_GLFormat = GL_RGB;
				m_GLType = GL_HALF_FLOAT;
				m_Xres = spec.width;
				m_Yres = spec.height;
				m_Channels = spec.nchannels;
				m_Size = m_Xres * m_Yres * m_Channels;
			}

			in->close();
		}

		void Release() noexcept;
		void Load(void* __restrict buffer, Profiler* prof) noexcept;
		void LoadExr(half* __restrict buffer) const noexcept;
		void LoadPng(uint8_t* __restrict buffer) const noexcept;
		void LoadJpg(uint8_t* __restrict buffer) const noexcept;
		void LoadOther(half* __restrict buffer) const noexcept;
		void GetTypeSize(uint8_t& img_format_size, uint8_t img_type_size) const noexcept;
	};

} // End namespace Core