// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "image.h"

namespace Core
{
	// releases an image data
	void Image::Release() noexcept
	{
		this->m_CacheIndex = 0;
	}

	ImVec4 Image::GetPixel(const uint16_t x, const uint16_t y, void* __restrict buffer) const noexcept
	{
		ImVec4 color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

		if (x < 0 || x >= this->m_Xres || y < 0 || y >= this->m_Yres) return color;

		if (this->m_Format & Format_RGBA_HALF)
		{
			const half* bufferCasted = (half*)buffer;
			const uint32_t index = x * 4 + this->m_Xres * 4 * y;

			color.x = static_cast<float>(bufferCasted[index + 0]);
			color.y = static_cast<float>(bufferCasted[index + 1]);
			color.z = static_cast<float>(bufferCasted[index + 2]);
			color.w = static_cast<float>(bufferCasted[index + 3]);
		}
		else if (this->m_Format & Format_RGB_U8)
		{
			const uint8_t* bufferCasted = (uint8_t*)buffer;
			const uint32_t index = x * 3 + this->m_Xres * 3 * y;

			color.x = static_cast<float>(bufferCasted[index + 0]) / 255.0f;
			color.y = static_cast<float>(bufferCasted[index + 1]) / 255.0f;
			color.z = static_cast<float>(bufferCasted[index + 2]) / 255.0f;
			color.w = 1.0f;
		}

		return color;
	}

	// loads an image
	void Image::LoadExr(half* __restrict buffer, const std::string& layerName) const noexcept
	{
		memset(&buffer[0], static_cast<half>(1.0f), this->m_Xres * this->m_Yres * (this->m_Channels > 4 ? 4 : this->m_Channels) * Size::Size16);
		
		// Read the RGBA layer, or more commonly the beauty
		if (layerName == "Beauty")
		{
			Imf::RgbaInputFile in(this->m_Path.c_str());
			
			const Imath::Box2i display = in.displayWindow();
			const Imath::Box2i data = in.dataWindow();
			const Imath::V2i dim(data.max.x - data.min.x + 1, data.max.y - data.max.y + 1);

			in.setFrameBuffer((Imf::Rgba*)buffer, 1, dim.x);
			in.readPixels(data.min.y, data.max.y);
		}
		// Read the specified layer
		else
		{
			Imf::InputFile in(this->m_Path.c_str());

			const Imath::Box2i display = in.header().displayWindow();
			const Imath::Box2i data = in.header().dataWindow();
			const Imath::V2i dim(data.max.x - data.min.x + 1, data.max.y - data.max.y + 1);
			
			Imf::FrameBuffer frameBuffer;

			// R channel
			char channelName[4096];
			Utils::Str::Format(channelName, "%s.R", layerName.c_str());

			frameBuffer.insert(channelName, Imf::Slice(Imf::HALF,
													   (char*) buffer,
													   sizeof(buffer[0]) * 1,
													   sizeof(buffer[0]) * this->m_Xres,
													   1, 1, 1.0));
			
			// G channel
			Utils::Str::Format(channelName, "%s.G", layerName.c_str());

			frameBuffer.insert(channelName, Imf::Slice(Imf::HALF,
													   (char*) buffer,
													   sizeof(buffer[0]) * 1,
													   sizeof(buffer[0]) * this->m_Xres,
													   1, 1, 1.0));
			
			// B channel
			Utils::Str::Format(channelName, "%s.B", layerName.c_str());

			frameBuffer.insert(channelName, Imf::Slice(Imf::HALF,
													   (char*) buffer,
													   sizeof(buffer[0]) * 1,
													   sizeof(buffer[0]) * this->m_Xres,
													   1, 1, 1.0));
			
			// A channel
			Utils::Str::Format(channelName, "%s.A", layerName.c_str());

			frameBuffer.insert(channelName, Imf::Slice(Imf::HALF,
													   (char*) buffer,
													   sizeof(buffer[0]) * 1,
													   sizeof(buffer[0]) * this->m_Xres,
													   1, 1, 1.0));

			in.setFrameBuffer(frameBuffer);
			in.readPixels(data.min.y, data.max.y);
		}
	}

	void Image::LoadPng(uint8_t* __restrict buffer) const noexcept
	{
		auto in = OIIO::ImageInput::open(this->m_Path);
		in->read_image(0, -1, OIIO::TypeDesc::UINT8, buffer);
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

	void Image::Load(void* __restrict buffer, Profiler* prof, const std::string& layerName) const noexcept
	{
		const auto load_timer_start = prof->Start();

		if (this->m_Type & FileType_Exr) LoadExr((half*)buffer, layerName);
		else if (this->m_Type& FileType_Jpg) LoadJpg((uint8_t*)buffer);
		else if (this->m_Type & FileType_Png) LoadPng((uint8_t*)buffer);
		else if (this->m_Type & FileType_Other) LoadOther((half*)buffer);

		const auto load_timer_end = prof->End();
		prof->Time("Image Loading Time", load_timer_start, load_timer_end);
	}
} // End namespace Core