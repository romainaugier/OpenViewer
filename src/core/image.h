// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include "stdint.h"
#include <filesystem>

#include "GL/glew.h"

#include "OpenImageIO/imageio.h"
#include "OpenEXR/ImfRgbaFile.h"
#include "OpenEXR/ImfInputFile.h"
#include "OpenEXR/ImfChannelList.h"
#include "OpenEXR/ImfMultiPartInputFile.h"

extern "C" {
	#include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>
	#include <libavutil/avutil.h>
	#include <libavutil/pixdesc.h>
	#include <libswscale/swscale.h>
}

#include "imgui.h"

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
	Format_RGB_HALF   = 0x80,
	Format_RGBA_U16   = 0x100,
	Format_RGB_U16    = 0x200,
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
		uint32_t m_CacheIndex = 0; // zero means it is not cached

		uint32_t m_Frame; // used for reading video streams
		
		OIIO::TypeDesc m_OIIOTypeDesc;

		uint8_t m_Depth = 0; // bit depth information
		
		FileType_ m_Type;
		Format_ m_Format;
		GLint m_GLInternalFormat;
		GLenum m_GLFormat;
		GLenum m_GLType;

		Image() {}

		Image(const std::string& fp)
		{
			this->m_Path = fp;
			
			auto in = OIIO::ImageInput::open(fp);
			
			if (!in)
			{
				StaticErrorConsoleLog(OIIO::geterror().c_str());
				return;
			}

			const OIIO::ImageSpec& spec = in->spec();

			if(Utils::Str::EndsWith(fp, ".exr"))
			{
				const bool hasAlpha = spec.alpha_channel > -1;
				
				this->m_Type = FileType_Exr;
				this->m_Xres = spec.full_width;
				this->m_Yres = spec.full_height;
				this->m_Channels = spec.nchannels;
				this->m_Size = this->m_Xres * this->m_Yres * (this->m_Channels > 4 ? 4 : this->m_Channels);

				if (spec.format == OIIO::TypeDesc::HALF)
				{
					this->m_OIIOTypeDesc = OIIO::TypeDesc::HALF;
					this->m_Depth = 2;
					this->m_Format = hasAlpha ? Format_RGBA_HALF : Format_RGB_HALF;
					this->m_GLInternalFormat = hasAlpha ? GL_RGBA16F : GL_RGB16F;
					this->m_GLFormat = hasAlpha ? GL_RGBA : GL_RGB;
					this->m_GLType = GL_HALF_FLOAT;
					this->m_Stride = m_Size * Size::Size16;
				}
				else if (spec.format == OIIO::TypeDesc::FLOAT)
				{
					this->m_OIIOTypeDesc = OIIO::TypeDesc::FLOAT;
					this->m_Depth = 4;
					this->m_Format = hasAlpha ? Format_RGBA_FLOAT : Format_RGB_FLOAT;
					this->m_GLInternalFormat = hasAlpha ? GL_RGBA32F : GL_RGB32F;
					this->m_GLFormat = hasAlpha ? GL_RGBA : GL_RGB;
					this->m_GLType = GL_FLOAT;
					this->m_Stride = m_Size * Size::Size32;
				}
				else
				{
					StaticErrorConsoleLog("Unknown data type in exr file : %s", this->m_Path);
					std::exit(EXIT_FAILURE);
				}

			}
			else if(Utils::Str::EndsWith(fp, ".png"))
			{
				const bool hasAlpha = spec.alpha_channel > -1;
				
				this->m_Type = FileType_Png;
				this->m_Xres = spec.width;
				this->m_Yres = spec.height;
				this->m_Channels = spec.nchannels;
				this->m_Size = m_Xres * m_Yres * this->m_Channels;
				
				if (spec.format == OIIO::TypeDesc::UINT8)
				{
					this->m_OIIOTypeDesc = OIIO::TypeDesc::UINT8;
					this->m_Depth = 1;
					this->m_Format = hasAlpha ? Format_RGBA_U8 : Format_RGB_U8;
					this->m_GLInternalFormat = hasAlpha ? GL_RGBA8 : GL_RGB8;
					this->m_GLFormat = hasAlpha ? GL_RGBA : GL_RGB;
					this->m_GLType = GL_UNSIGNED_BYTE;
					this->m_Stride = m_Size * Size::Size8;
				}
				else if (spec.format == OIIO::TypeDesc::UINT16)
				{
					this->m_OIIOTypeDesc = OIIO::TypeDesc::UINT16;
					this->m_Depth = 2;
					this->m_Format = hasAlpha ? Format_RGBA_U16 : Format_RGB_U16;
					this->m_GLInternalFormat = hasAlpha ? GL_RGBA16 : GL_RGB16;
					this->m_GLFormat = hasAlpha ? GL_RGBA : GL_RGB;
					this->m_GLType = GL_UNSIGNED_SHORT;
					this->m_Stride = m_Size * Size::Size16;
				}
				else
				{
					StaticErrorConsoleLog("Unknown data type in png file : %s", this->m_Path);
					std::exit(EXIT_FAILURE);
				}
			}
			else if(Utils::Str::EndsWith(fp, ".jpg") || Utils::Str::EndsWith(fp, ".jpeg"))
			{
				this->m_OIIOTypeDesc = OIIO::TypeDesc::UINT8;
				this->m_Type = FileType_Jpg;
				this->m_Format = Format_RGB_U8;
				this->m_GLInternalFormat = GL_RGB8;
				this->m_GLFormat = GL_RGB;
				this->m_GLType = GL_UNSIGNED_BYTE;
				this->m_Xres = spec.width;
				this->m_Yres = spec.height;
				this->m_Channels = spec.nchannels;
				this->m_Size = m_Xres * m_Yres * m_Channels;
				this->m_Stride = m_Size * Size::Size8;
				this->m_Depth = 1;
			}
			else if(Utils::Str::EndsWith(fp, ".mp4"))
			{
				this->m_Type = FileType_Mp4;
				this->m_Xres = spec.width;
				this->m_Yres = spec.height;
				this->m_Channels = spec.nchannels;
			}
			else if(Utils::Str::EndsWith(fp, ".mov"))
			{
				this->m_Type = FileType_Mov;
				this->m_Xres = spec.width;
				this->m_Yres = spec.height;
				this->m_Channels = spec.nchannels;
			}
			else if(Utils::Str::EndsWith(fp, ".hdr"))
			{
				this->m_OIIOTypeDesc = OIIO::TypeDesc::FLOAT;
				this->m_Type = FileType_Hdr;
				this->m_Format = Format_RGB_FLOAT;
				this->m_GLInternalFormat = GL_RGB32F;
				this->m_GLFormat = GL_RGB;
				this->m_GLType = GL_FLOAT;
				this->m_Xres = spec.width;
				this->m_Yres = spec.height;
				this->m_Channels = spec.nchannels;
				this->m_Size = m_Xres * m_Yres * m_Channels;
				this->m_Stride = m_Size * Size::Size32;
				this->m_Depth = 4;
			}
			else if(Utils::Str::EndsWith(fp, ".tiff") || Utils::Str::EndsWith(fp, ".tif"))
			{
				this->m_Type = FileType_Tiff;
				this->m_Xres = spec.width;
				this->m_Yres = spec.height;
				this->m_Channels = spec.nchannels;
				this->m_Size = m_Xres * m_Yres * m_Channels;

				if (spec.format == OIIO::TypeDesc::UINT8)
				{
					this->m_Depth = 1;
					this->m_Format = Format_RGB_U8;
					this->m_GLInternalFormat = GL_RGB8;
					this->m_GLFormat = GL_RGB;
					this->m_GLType = GL_UNSIGNED_BYTE;
					this->m_Stride = m_Size * Size::Size8;
				}
				else if (spec.format == OIIO::TypeDesc::UINT16)
				{
					this->m_Depth = 2;
					this->m_Format = Format_RGB_U16;
					this->m_GLInternalFormat = GL_RGB16;
					this->m_GLFormat = GL_RGB;
					this->m_GLType = GL_UNSIGNED_SHORT;
					this->m_Stride = m_Size * Size::Size16;
				}
				else if (spec.format == OIIO::TypeDesc::FLOAT)
				{
					this->m_Depth = 4;
					this->m_Format = Format_RGB_FLOAT;
					this->m_GLInternalFormat = GL_RGB32F;
					this->m_GLFormat = GL_RGB;
					this->m_GLType = GL_FLOAT;
					this->m_Stride = m_Size * Size::Size32;
				}

				else
				{
					StaticErrorConsoleLog("Unknown data type in tiff file : %s", this->m_Path);
					std::exit(EXIT_FAILURE);
				}
			}
			else
			{
				this->m_Type = FileType_Other;
				this->m_Format = Format_RGB_HALF;
				this->m_GLInternalFormat = GL_RGB16F;
				this->m_GLFormat = GL_RGB;
				this->m_GLType = GL_HALF_FLOAT;
				this->m_Xres = spec.width;
				this->m_Yres = spec.height;
				this->m_Channels = spec.nchannels;
				this->m_Size = m_Xres * m_Yres * m_Channels;
				this->m_Stride = this->m_Size * Size::Size16;
				this->m_Depth = 2;
			}

			in->close();
		}

		void Release() noexcept;
		
		void Load(void* __restrict buffer, Profiler* prof, const std::string& layers = "") const noexcept;
		
		void LoadExr(void* __restrict buffer, const std::string& layers = "") const noexcept;

		// When reading multichannels exr, the channels format and type can vary (half, float, rgb, rbga) so we make sure to 
		// set the good format in order to read it correctly 
		void VerifyChannelSize(const std::string& layers = "") noexcept;
		
		void LoadPng(void* __restrict buffer) const noexcept;
		
		void LoadJpg(void* __restrict buffer) const noexcept;

		void LoadHdr(void* __restrict buffer) const noexcept;

		void LoadTiff(void* __restrict buffer) const noexcept;
		
		void LoadOther(void* __restrict buffer) const noexcept;

		ImVec4 GetPixel(const uint16_t x, const uint16_t y, void* __restrict buffer) const noexcept;
	};

} // End namespace Core