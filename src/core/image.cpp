// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "image.h"

namespace Core
{
	// releases an image data
	void Image::Release() noexcept
	{
		this->m_CacheIndex = -1;
	}

	// returns the size of the image type (float, uint8...)
	void Image::GetTypeSize(uint8_t& img_format_size, uint8_t img_type_size) const noexcept
	{
		// default format will be RGBA (4 channels) as half float (16 bits)
		// as I assume most images will be exr with alpha
		img_format_size = SIZEOF_HALF_FLOAT * 4;
		img_type_size = SIZEOF_HALF_FLOAT;

		// other formats will be "detected" here
		// we make a big branch to avoid having too much branching due to 
		// the format testing
		if(!this->m_Format & Format_RGB_HALF)
		{
			if(this->m_Format & Format_RGB_U8) 
			{ 
				img_format_size = 3 * SIZEOF_UINT8;
				img_type_size = SIZEOF_UINT8;
			}
			else if(this->m_Format & Format_RGBA_U8) 
			{
				img_format_size = 4 * SIZEOF_UINT8;
				img_type_size = SIZEOF_UINT8;
			}
			else if(this->m_Format & Format_RGB_FLOAT)
			{
				img_format_size = 3 * SIZEOF_FLOAT; 
				img_type_size = SIZEOF_FLOAT;
			}
			else if(this->m_Format & Format_RGBA_FLOAT) 
			{
				img_format_size = 4 * SIZEOF_FLOAT; 
				img_type_size = SIZEOF_FLOAT;
			}
			else if(this->m_Format & Format_RGB_HALF) 
			{
				img_format_size = 3 * SIZEOF_HALF_FLOAT;
				img_type_size = SIZEOF_HALF_FLOAT;
			}
		}
	}

	// loads an image
	void Image::LoadExr(half* __restrict buffer) const noexcept
	{
		Imf::RgbaInputFile in(this->m_Path.c_str());
		
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
			memset(&buffer[0], static_cast<half>(0.0f), this->m_Xres * this->m_Yres * this->m_Channels);
		}

		in.setFrameBuffer((Imf::Rgba*)buffer, 1, dim.x);
		in.readPixels(data.min.y, data.max.y);
	}

	void Image::LoadPng(uint8_t* __restrict buffer) const noexcept
	{
		auto in = OIIO::ImageInput::open(this->m_Path);
		in->read_image(0, -1, OIIO::TypeDesc::UINT8, (uint8_t*)buffer);
		in->close();
	}

	void Image::LoadJpg(uint8_t* __restrict buffer) const noexcept
	{
		auto in = OIIO::ImageInput::open(this->m_Path);
		in->read_image(0, -1, OIIO::TypeDesc::UINT8, (uint8_t*)buffer);
		in->close();
	}

	void Image::LoadOther(half* __restrict buffer) const noexcept
	{
		auto in = OIIO::ImageInput::open(this->m_Path);
		in->read_image(0, -1, OIIO::TypeDesc::HALF, (half*)buffer);
		in->close();
	}

	void Image::Load(void* __restrict buffer, Profiler* prof) noexcept
	{
		
		auto load_timer_start = prof->Start();

		if (this->m_Type & FileType_Exr) LoadExr((half*)buffer);
		if (this->m_Type& FileType_Jpg) LoadJpg((uint8_t*)buffer);
		if (this->m_Type & FileType_Png) LoadPng((uint8_t*)buffer);
		else if (this->m_Type & FileType_Other) LoadOther((half*)buffer);

		auto load_timer_end = prof->End();
		prof->Time("Image Loading Time", load_timer_start, load_timer_end);
	}
} // End namespace Core