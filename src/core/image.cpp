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
		else if (this->m_Format & Format_RGB_HALF)
		{
			const half* bufferCasted = (half*)buffer;
			const uint32_t index = x * 3 + this->m_Xres * 3 * y;

			color.x = static_cast<float>(bufferCasted[index + 0]);
			color.y = static_cast<float>(bufferCasted[index + 1]);
			color.z = static_cast<float>(bufferCasted[index + 2]);
			color.w = 1.0f;
		}
		else if (this->m_Format & Format_RGBA_FLOAT)
		{
			const float* bufferCasted = (float*)buffer;
			const uint32_t index = x * 4 + this->m_Xres * 4 * y;

			color.x = bufferCasted[index + 0];
			color.y = bufferCasted[index + 1];
			color.z = bufferCasted[index + 2];
			color.w = bufferCasted[index + 3];
		}
		else if (this->m_Format & Format_RGB_FLOAT)
		{
			const float* bufferCasted = (float*)buffer;
			const uint32_t index = x * 3 + this->m_Xres * 3 * y;

			color.x = bufferCasted[index + 0];
			color.y = bufferCasted[index + 1];
			color.z = bufferCasted[index + 2];
			color.w = 1.0f;
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
		else if (this->m_Format & Format_RGBA_U8)
		{
			const uint8_t* bufferCasted = (uint8_t*)buffer;
			const uint32_t index = x * 4 + this->m_Xres * 4 * y;

			color.x = static_cast<float>(bufferCasted[index + 0]) / 255.0f;
			color.y = static_cast<float>(bufferCasted[index + 1]) / 255.0f;
			color.z = static_cast<float>(bufferCasted[index + 2]) / 255.0f;
			color.w = static_cast<float>(bufferCasted[index + 3]) / 255.0f;
		}

		return color;
	}

	// loads an image
	void Image::LoadExr(void* __restrict buffer, const std::string& layers, const uint8_t exrThreads) const noexcept
	{
		std::vector<std::string> channelNames;
		Utils::Str::Split(channelNames, layers, ';');

		Imf::MultiPartInputFile in(this->m_Path.c_str(), exrThreads);

		int partIdx = 0;

		for (uint16_t i = 0; i < in.parts(); i++)
		{
			if (in.header(i).channels().findChannel(channelNames[0]) != nullptr) partIdx = i;
		}

		const Imath::Box2i display = in.header(0).displayWindow();
		const Imath::Box2i data = in.header(0).dataWindow();
		const Imath::V2i dim(data.max.x - data.min.x + 1, data.max.y - data.max.y + 1);

		const int dx = data.min.x;
		const int dy = data.min.y;
		
		Imf::FrameBuffer frameBuffer;

		uint8_t strideMultiplier = 4;
		uint8_t strideOffset = 0;
		uint8_t dataSize = Size::Size16; // Half by default
		Imf::PixelType pixelType = Imf::HALF; // Half by default

		half* bufferCastedHalf = static_cast<half*>(buffer);
		float* bufferCastedFloat = (float*)buffer;

		if (this->m_Format & Format_RGB_HALF || this->m_Format & Format_RGB_FLOAT)
		{
			strideMultiplier = 3;
		}

		if (this->m_Format & Format_RGB_FLOAT || this->m_Format & Format_RGBA_FLOAT)
		{
			dataSize = Size::Size32;
			pixelType = Imf::FLOAT;
		}
		
		if (data.max.x < display.max.x || data.max.y < display.max.y ||
			data.min.x > display.min.x || data.min.y > display.min.y)
		{
			memset(buffer, 0.0f, this->m_Xres * this->m_Yres * (channelNames.size() < 3 ? 3 : channelNames.size()) * dataSize);
		}
		
		for (auto& channelName : channelNames)
		{
			if (channelName == "") continue;
			
			if (pixelType == Imf::HALF)
			frameBuffer.insert(channelName, Imf::Slice(pixelType,
													   (char*) &bufferCastedHalf[strideOffset],
													   dataSize * strideMultiplier,
													   dataSize * this->m_Xres * strideMultiplier,
													   1, 1, 1.0));
			else if (pixelType == Imf::FLOAT)
			frameBuffer.insert(channelName, Imf::Slice(pixelType,
													   (char*) &bufferCastedFloat[strideOffset],
													   dataSize * strideMultiplier,
													   dataSize * this->m_Xres * strideMultiplier,
													   1, 1, 1.0));

			++strideOffset;
		}

		Imf::InputPart inputPart(in, partIdx);

		inputPart.setFrameBuffer(frameBuffer);
		inputPart.readPixels(data.min.y, data.max.y);

		// When we only have one channel, for example the z depth, we need to copy it to the other channels G and B
		if (channelNames.size() < 3)
		{
			if (pixelType == Imf::FLOAT)
			{
				for (uint32_t y = 0; y < this->m_Yres; y++)
				{
					for (uint32_t x = 0; x < this->m_Xres * 3; x += 3)
					{
						float rVal = bufferCastedFloat[x + y * this->m_Xres * 3];
						bufferCastedFloat[x + 1 + y * this->m_Xres * 3] = rVal;
						bufferCastedFloat[x + 2 + y * this->m_Xres * 3] = rVal;
					}
				}
			}
			else if (pixelType == Imf::HALF)
			{
				for (uint32_t y = 0; y < this->m_Yres; y++)
				{
					for (uint32_t x = 0; x < this->m_Xres * 3; x += 3)
					{
						float rVal = bufferCastedHalf[x + y * this->m_Xres * 3];
						bufferCastedHalf[x + 1 + y * this->m_Xres * 3] = rVal;
						bufferCastedHalf[x + 2 + y * this->m_Xres * 3] = rVal;
					}
				}
			}
		}
	}

	void Image::VerifyChannelSize(const std::string& layers) noexcept
	{
		if (this->m_Type & FileType_Exr)
		{
			std::vector<std::string> channelNames;

			Utils::Str::Split(channelNames, layers, ';');

			const uint8_t channelCount = channelNames.size() >= 3 ? channelNames.size() : 3;

			Imf::MultiPartInputFile in(this->m_Path.c_str());

			uint8_t channelPixType;
			
			for (uint32_t i = 0; i < in.parts(); i++)
			{
				if (in.header(i).channels().findChannel(channelNames[0]) != nullptr)
				{
					channelPixType = in.header(i).channels().find(channelNames[0]).channel().type;
				}
			}

			if (channelPixType == 1)
			{
				if (channelCount == 3 && !(this->m_Format & Format_RGB_HALF))
				{
					this->m_Format = Format_RGB_HALF;
					this->m_GLInternalFormat = GL_RGB16F;
					this->m_GLFormat = GL_RGB;
					this->m_GLType = GL_HALF_FLOAT;
					this->m_Stride = this->m_Xres * this->m_Yres * 3 * Size::Size16;
					this->m_Channels = 3;
				}
				else if (channelCount == 4 && !(this->m_Format & Format_RGBA_HALF))
				{
					this->m_Format = Format_RGBA_HALF;
					this->m_GLInternalFormat = GL_RGBA16F;
					this->m_GLFormat = GL_RGBA;
					this->m_GLType = GL_HALF_FLOAT;
					this->m_Stride = this->m_Xres * this->m_Yres * 4 * Size::Size16;
					this->m_Channels = 4;
				}

				this->m_Depth = 2;
			}
			else if (channelPixType == 2)
			{
				if (channelCount == 3 && !(this->m_Format & Format_RGB_FLOAT))
				{
					this->m_Format = Format_RGB_FLOAT;
					this->m_GLInternalFormat = GL_RGB32F;
					this->m_GLFormat = GL_RGB;
					this->m_GLType = GL_FLOAT;
					this->m_Stride = this->m_Xres * this->m_Yres * 3 * Size::Size32;
					this->m_Channels = 3;
				}
				else if (channelCount == 4 && !(this->m_Format & Format_RGBA_FLOAT))
				{
					this->m_Format = Format_RGBA_FLOAT;
					this->m_GLInternalFormat = GL_RGBA32F;
					this->m_GLFormat = GL_RGBA;
					this->m_GLType = GL_FLOAT;
					this->m_Stride = this->m_Xres * this->m_Yres * 4 * Size::Size32;
					this->m_Channels = 4;
				}

				this->m_Depth = 4;
			}
		}
	}

	void Image::LoadPng(void* __restrict buffer) const noexcept
	{
		auto in = OIIO::ImageInput::open(this->m_Path);
		in->read_image(0, -1, this->m_OIIOTypeDesc, buffer);
		in->close();
	}

	void Image::LoadJpg(void* __restrict buffer) const noexcept
	{
		auto in = OIIO::ImageInput::open(this->m_Path);
		in->read_image(0, -1, this->m_OIIOTypeDesc, (uint8_t*)buffer);
		in->close();
	}

	void Image::LoadHdr(void* __restrict buffer) const noexcept
	{
		auto in = OIIO::ImageInput::open(this->m_Path);
		in->read_image(0, -1, this->m_OIIOTypeDesc, (float*)buffer);
		in->close();
	}

	void Image::LoadTiff(void* __restrict buffer) const noexcept
	{
		auto in = OIIO::ImageInput::open(this->m_Path);
		in->read_image(0, -1, this->m_OIIOTypeDesc, buffer);
		in->close();
	}

	void Image::LoadOther(void* __restrict buffer) const noexcept
	{
		auto in = OIIO::ImageInput::open(this->m_Path);
		in->read_image(0, -1, OIIO::TypeDesc::HALF, buffer);
		in->close();
	}

	void Image::Load(void* __restrict buffer) const noexcept
	{
		// if (this->m_Type & FileType_Exr) LoadExr(buffer);
		if (this->m_Type & FileType_Jpg) LoadJpg(buffer);
		else if (this->m_Type & FileType_Png) LoadPng(buffer);
		else if (this->m_Type & FileType_Hdr) LoadHdr(buffer);
		else if (this->m_Type & FileType_Tiff) LoadTiff(buffer);
		else if (this->m_Type & FileType_Other) LoadOther(buffer);
	}
} // End namespace Core