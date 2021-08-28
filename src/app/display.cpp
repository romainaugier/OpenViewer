// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#include "display.h"

// Converts the half buffer to a float buffer, in order to be able to be processed by the OCIO processor
void OPENVIEWER_VECTORCALL Display::Unpack(const half* __restrict half_buffer, const int64_t size, bool add_alpha) noexcept
{
	if (!add_alpha)
	{ 
		for (int64_t i = 0; i < size; i += 8)
		{
			__m128i arr = _mm_lddqu_si128((__m128i*) & half_buffer[i]);
			__m256 floats = _mm256_cvtph_ps(arr);
			_mm256_store_ps(&buffer[i], floats);
		}
	}
	// unpack and then we set the alpha to 1.0f
	else
	{
		int64_t idx = 0;

		for (int64_t i = 0; i < size; i += 3)
		{
			__m128i arr = _mm_lddqu_si128((__m128i*) & half_buffer[i]);
			__m128 floats = _mm_cvtph_ps(arr);
			_mm_store_ps(&buffer[idx], floats);
			
			buffer[idx + 3] = 1.0f;
			idx += 4;
		}
	}
}

void Display::InitializeOpenGL(const Image& img) noexcept
{
	// Generate the frame buffer object
	glGenFramebuffers(1, &fbo);

	// Generate the color attachement texture
	glGenTextures(1, &tex_color_buffer);
	glBindTexture(GL_TEXTURE_2D, tex_color_buffer);
	glTexImage2D(GL_TEXTURE_2D, 0, img.internal_format, width, height, 0, img.gl_format, img.gl_type, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Generate the render buffer object
	glGenRenderbuffers(1, &rbo);
	BindRBO();
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	// Attach the texture and the frame buffer
	BindFBO();
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_color_buffer, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	// verify that the framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) printf("OpenGL Error : Framebuffer is not complete");

	UnbindFBO();
	UnbindRBO();
}

// Binds the display frame buffer object
OPENVIEWER_FORCEINLINE void Display::BindFBO() const noexcept
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

// Unbinds the display frame buffer object
OPENVIEWER_FORCEINLINE void Display::UnbindFBO() const noexcept
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Binds the ddisplay render buffer object
OPENVIEWER_FORCEINLINE void Display::BindRBO() const noexcept
{
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
}

// Unbinds the display render buffer object
OPENVIEWER_FORCEINLINE void Display::UnbindRBO() const noexcept
{
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

// Initializes the gl texture that will display the images
void Display::Initialize(const Loader& loader, Ocio& ocio) noexcept
{
	display = 1;

	bool set_alpha = false;

	const uint64_t size = loader.images[0].size;
	width = loader.images[0].xres;
	height = loader.images[0].yres;

	mipmap_idx = floor(height / 1000);

	if (size < (width * height * 4))
	{
		// we need to resize the buffer to support 4 channels : alpha, and rgb
		set_alpha = true;
		buffer = static_cast<float*>(OvAlloc(width * height * 4 * sizeof(float), 32));
		buffer_size = width * height * 4 * sizeof(float);
	}
	else
	{
		buffer = static_cast<float*>(OvAlloc(size * sizeof(float), 32));
		buffer_size = size * sizeof(float);
	}

	InitializeOpenGL(loader.images[0]);

	use_buffer = 1;


	auto plot_start = profiler->Start();
	Unpack((half*)loader.memory_arena, size, set_alpha);
	auto plot_end = profiler->End();
	profiler->Time("Image Unpacking Time", plot_start, plot_end);

	glGenTextures(1, &display_tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, display_tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glTexImage2D(GL_TEXTURE_2D, 0, loader.images[0].internal_format, width, height, 0, 
						  loader.images[0].gl_format, loader.images[0].gl_type, buffer);

	// OCIO GPU Processing
	if (ocio.use_gpu > 0)
	{
		auto ocio_start = profiler->Start();

		// Bind the framebuffer
		BindFBO();
		glViewport(0, 0, width, height);
		glEnable(GL_DEPTH_TEST);

		// Clear the framebuffer
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ocio.UpdateProcessor();
		ocio.Process(buffer, width, height);

		// Draw the quad
		glEnable(GL_TEXTURE_2D);

		glPushMatrix();
			glBegin(GL_QUADS);
				glTexCoord2f(1.0f, 1.0f);
				glVertex2f(1.0f, 1.0f);

				glTexCoord2f(1.0f, 0.0f);
				glVertex2f(1.0f, -1.0f);

				glTexCoord2f(0.0f, 0.0f);
				glVertex2f(-1.0f, -1.0f);

				glTexCoord2f(0.0f, 1.0f);
				glVertex2f(-1.0f, 1.0f);
			glEnd();
		glPopMatrix();

		glDisable(GL_TEXTURE_2D);

		// Unbind frame buffer object and clear
		UnbindFBO();
		glDisable(GL_DEPTH_TEST);

		auto ocio_end = profiler->End();
		profiler->Time("Ocio Transform Time", ocio_start, ocio_end);
	}

	// Unbind our texture
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
	

	UnbindFBO();
}

// Updates the gl texture that displays the images
void Display::Update(const Loader& loader, Ocio& ocio, const uint16_t frame_idx) noexcept
{
	if (display > 0)
	{
		const uint16_t xres = loader.images[frame_idx].xres;
		const uint16_t yres = loader.images[frame_idx].yres;
		const int64_t size = loader.images[frame_idx].size;
		const void* address = loader.images[frame_idx].cache_address;

		if (address == nullptr)
		{
			StaticDebugConsoleLog("Display Error : Frame Index %d", frame_idx);
			StaticDebugConsoleLog("Previous frame cache : %d", loader.cached[frame_idx - 1]);
			StaticDebugConsoleLog("Next frame cache : %d", loader.cached[frame_idx + 1]);
		}

		bool set_alpha = false;

		if (size < (xres * yres * 4)) set_alpha = true;

		auto plot_start = profiler->Start();
		Unpack((half*)address, size, set_alpha);
		auto plot_end = profiler->End();
		profiler->Time("Image Unpacking Time", plot_start, plot_end);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, display_tex);
		GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D,
			0, 0, 0,
			xres,
			yres,
			loader.images[frame_idx].gl_format,
			loader.images[frame_idx].gl_type,
			buffer));

		// OCIO GPU Processing
		if (ocio.use_gpu > 0)
		{
			auto ocio_start = profiler->Start();

			// Bind the framebuffer
			BindFBO();
			glViewport(0, 0, xres, yres);
			glEnable(GL_DEPTH_TEST);

			// Clear the framebuffer
			glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			ocio.Process(buffer, xres, yres);

			// Draw the quad
			glEnable(GL_TEXTURE_2D);

			glPushMatrix();
				glBegin(GL_QUADS);
					glTexCoord2f(1.0f, 1.0f);
					glVertex2f(1.0f, 1.0f);

					glTexCoord2f(1.0f, 0.0f);
					glVertex2f(1.0f, -1.0f);

					glTexCoord2f(0.0f, 0.0f);
					glVertex2f(-1.0f, -1.0f);

					glTexCoord2f(0.0f, 1.0f);
					glVertex2f(-1.0f, 1.0f);
				glEnd();
			glPopMatrix();

			glDisable(GL_TEXTURE_2D);

			// Unbind frame buffer object and clear
			UnbindFBO();
			glDisable(GL_DEPTH_TEST);

			auto ocio_end = profiler->End();
			profiler->Time("Ocio Transform Time", ocio_start, ocio_end);

			// Unbind our texture
			GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
		}
	}
}

// Main function that contains the window drawing 
void Display::Draw(Loader& loader, uint16_t frame_idx) const noexcept
{
	/*
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize;
	*/

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoCollapse;

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
			ImGui::Image((void*)(intptr_t)tex_color_buffer, size * zoom, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), tint, border_color);
		}
	}
	ImGui::End();
}

void Display::GetDisplayPixels() noexcept
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
	glBindTexture(GL_TEXTURE_2D, tex_color_buffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	glGetTexImage(GL_TEXTURE_2D, mipmap_idx, GL_RGBA, GL_FLOAT, buffer);
	glBindTexture(GL_TEXTURE_2D, 0);
	UnbindFBO();
}

void Display::Release() noexcept
{
	// release the display image buffer
	if (use_buffer > 0)
	{
		OvFree(buffer);
		buffer = nullptr;
	}

	// delete the gl buffers
	glDeleteFramebuffers(1, &fbo);
	glDeleteRenderbuffers(1, &rbo);

	// delete the gl textures
	glDeleteTextures(1, &display_tex);
	glDeleteTextures(1, &tex_color_buffer);
}