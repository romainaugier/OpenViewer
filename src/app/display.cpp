// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#include "display.h"

// Converts the half buffer to a float buffer, in order to be able to be processed by the OCIO processor
void __vectorcall Display::ToFloat(const half* __restrict half_buffer, const int64_t size) noexcept
{
	for (int64_t i = 0; i < size; i += 8)
	{
		__m128i arr = _mm_lddqu_si128((__m128i*)&half_buffer[i]);
		__m256 floats = _mm256_cvtph_ps(arr);
		_mm256_store_ps(&buffer[i], floats);
	}
}

// Initializes the gl texture that will display the images
void Display::Initialize(const Loader& loader, Ocio& ocio) noexcept
{
	display = 1;

	if (loader.images[0].type & FileType_Exr)
	{
		const uint64_t size = loader.images[0].size;
		const uint16_t xres = loader.images[0].xres;
		const uint16_t yres = loader.images[0].yres;

		use_buffer = 1;

		buffer = (float*)_aligned_malloc(size * sizeof(float), 32);

		ToFloat((half*)loader.memory_arena, size);

		ocio.Process(buffer, xres, yres); 

		glGenTextures(1, &display_tex);
		glBindTexture(GL_TEXTURE_2D, display_tex);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_RGBA,
			xres,
			yres,
			0,
			GL_RGBA,
			GL_FLOAT,
			buffer);

		glBindTexture(GL_TEXTURE_2D, 0);
		
	}
	else
	{
		glGenTextures(1, &display_tex);
		glBindTexture(GL_TEXTURE_2D, display_tex);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		glTexImage2D(GL_TEXTURE_2D,
			0,
			loader.images[0].internal_format,
			loader.images[0].xres,
			loader.images[0].yres,
			0,
			loader.images[0].gl_format,
			loader.images[0].gl_type,
			loader.memory_arena);

		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

// Updates the gl texture that displays the images
void Display::Update(const Loader& loader, Ocio& ocio, const uint16_t frame_idx) noexcept
{
	if (display > 0)
	{
		const uint16_t xres = loader.images[frame_idx].xres;
		const uint16_t yres = loader.images[frame_idx].yres;

		if (loader.images[frame_idx].type & FileType_Exr)
		{
			const int64_t size = loader.images[frame_idx].size;

			ToFloat((half*)loader.memory_arena, size);

			ocio.Process(buffer, xres, yres);

			glBindTexture(GL_TEXTURE_2D, display_tex);
			glTexSubImage2D(GL_TEXTURE_2D,
				0, 0, 0,
				xres,
				yres,
				GL_RGBA,
				GL_FLOAT,
				buffer);

			glBindTexture(GL_TEXTURE_2D, 0);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, display_tex);
			glTexSubImage2D(GL_TEXTURE_2D,
				0, 0, 0,
				xres,
				yres,
				loader.images[frame_idx].gl_format,
				loader.images[frame_idx].gl_type,
				loader.memory_arena);

			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
}

// Main function that contains the window drawing 
void Display::Draw(Loader& loader, uint16_t frame_idx) const noexcept
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize;

	bool p_open = true;

	ImGui::Begin("Display", &p_open, window_flags);
	{
		if (display > 0)
		{
			ImVec2 size = ImVec2(loader.images[frame_idx].xres, loader.images[frame_idx].yres);

			static ImVec2 scrolling;
			static float zoom = 1.0f;
			const ImVec4 tint(1.0f, 1.0f, 1.0f, 1.0f);
			const ImVec4 border_color(0.0f, 0.0f, 0.0f, 1.0f);

			const bool is_hovered = ImGui::IsWindowHovered();

			ImGuiIO& io = ImGui::GetIO();

			if (is_hovered && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			{
				scrolling.x += io.MouseDelta.x;
				scrolling.y += io.MouseDelta.y;
			}

			const float mouse_wheel = io.MouseWheel;

			if (is_hovered && mouse_wheel != 0)
			{
				zoom += mouse_wheel * 0.1f;
			}

			ImGui::SetCursorPos((ImGui::GetWindowSize() - size * zoom) * 0.5f + scrolling);
			ImGui::Image((void*)(intptr_t)display_tex, size * zoom, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), tint, border_color);
		}
	}
	ImGui::End();
}

void Display::Release() noexcept
{
	if (use_buffer > 0)
	{
		_aligned_free(buffer);
		buffer = nullptr;
	}
}