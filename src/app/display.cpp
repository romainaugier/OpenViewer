// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#include "display.h"

// Converts the half buffer to a float buffer, in order to be able to be processed by the OCIO processor
void __vectorcall Display::ToFloat(const half* __restrict half_buffer, const int64_t size) noexcept
{
	for (int64_t i = 0; i < size; i += 8)
	{
		__m128i arr = _mm_lddqu_si128((__m128i*) & half_buffer[i]);
		__m256 floats = _mm256_cvtph_ps(arr);
		_mm256_store_ps(&buffer[i], floats);
	}
}

void Display::InitializeOpenGL(const uint16_t width, const uint16_t height) noexcept
{
	// Generate the frame buffer object
	glGenFramebuffers(1, &fbo);

	// Generate the color attachement texture
	glGenTextures(1, &tex_color_buffer);
	glBindTexture(GL_TEXTURE_2D, tex_color_buffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
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
__forceinline void Display::BindFBO() const noexcept
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

// Unbinds the display frame buffer object
__forceinline void Display::UnbindFBO() const noexcept
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Binds the ddisplay render buffer object
__forceinline void Display::BindRBO() const noexcept
{
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
}

// Unbinds the display render buffer object
__forceinline void Display::UnbindRBO() const noexcept
{
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

// Initializes the gl texture that will display the images
void Display::Initialize(const Loader& loader, Ocio& ocio, Profiler& prof) noexcept
{
	display = 1;

	const uint64_t size = loader.images[0].size;
	const uint16_t xres = loader.images[0].xres;
	const uint16_t yres = loader.images[0].yres;

	InitializeOpenGL(xres, yres);

	if (loader.images[0].type & FileType_Exr)
	{
		use_buffer = 1;

		buffer = (float*)_aligned_malloc(size * sizeof(float), 32);

		auto plot_start = prof.Start();
		ToFloat((half*)loader.memory_arena, size);
		auto plot_end = prof.End();
		prof.Plot(plot_start, plot_end);

		glGenTextures(1, &display_tex);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, display_tex);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, xres, yres, 0, GL_RGBA, GL_FLOAT, buffer));

		// OCIO GPU Processing
		if (ocio.use_gpu > 0)
		{
			auto ocio_start = prof.Start();

			// Bind the framebuffer
			BindFBO();
			glViewport(0, 0, xres, yres);
			glEnable(GL_DEPTH_TEST);

			// Clear the framebuffer
			glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			ocio.UpdateProcessor();
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

			auto ocio_end = prof.End();
			prof.Ocio(ocio_start, ocio_end);
		}

		// Unbind our texture
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
	}
	else
	{
		glGenTextures(1, &display_tex);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, display_tex);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		GL_CHECK(glTexImage2D(GL_TEXTURE_2D,
			0,
			loader.images[0].internal_format,
			xres,
			yres,
			0,
			loader.images[0].gl_format,
			loader.images[0].gl_type,
			loader.memory_arena));

		GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
	}

	UnbindFBO();
}

// Updates the gl texture that displays the images
void Display::Update(const Loader& loader, Ocio& ocio, const uint16_t frame_idx, Profiler& prof) noexcept
{
	if (display > 0)
	{
		const uint16_t xres = loader.images[frame_idx].xres;
		const uint16_t yres = loader.images[frame_idx].yres;

		if (loader.images[frame_idx].type & FileType_Exr)
		{
			const int64_t size = loader.images[frame_idx].size;

			auto plot_start = prof.Start();
			ToFloat((half*)loader.images[frame_idx].cache_address, size);
			auto plot_end = prof.End();
			prof.Plot(plot_start, plot_end);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, display_tex);
			GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D,
				0, 0, 0,
				xres,
				yres,
				GL_RGBA,
				GL_FLOAT,
				buffer));

			// OCIO GPU Processing
			if (ocio.use_gpu > 0)
			{
				auto ocio_start = prof.Start();

				// Bind the framebuffer
				BindFBO();
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

				auto ocio_end = prof.End();
				prof.Ocio(ocio_start, ocio_end);
			}

			// Unbind our texture
			GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
		}
		else
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, display_tex);
			GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D,
				0, 0, 0,
				xres,
				yres,
				loader.images[frame_idx].gl_format,
				loader.images[frame_idx].gl_type,
				loader.images[frame_idx].cache_address));

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

void Display::Release() noexcept
{
	// release the display image buffer
	if (use_buffer > 0)
	{
		_aligned_free(buffer);
		buffer = nullptr;
	}

	// delete the gl buffers
	glDeleteFramebuffers(1, &fbo);
	glDeleteRenderbuffers(1, &rbo);

	// delete the gl textures
	glDeleteTextures(1, &display_tex);
	glDeleteTextures(1, &tex_color_buffer);
}