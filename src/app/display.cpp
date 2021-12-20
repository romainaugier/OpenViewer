// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "display.h"

namespace Interface
{
	void Display::InitializeOpenGL(const Core::Image& image) noexcept
	{
		// Generate the frame buffer object
		glGenFramebuffers(1, &this->m_FBO);

		// Generate the color attachement texture
		glGenTextures(1, &this->m_ColorBuffer);
		glBindTexture(GL_TEXTURE_2D, this->m_ColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 
					 0, 
					 image.m_GLInternalFormat,
					 this->m_Width, 
					 this->m_Height, 
					 0, 
					 image.m_GLFormat, 
					 image.m_GLType, 
					 nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Generate the render buffer object
		glGenRenderbuffers(1, &this->m_RBO);
		BindRBO();
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, this->m_Width, this->m_Height);

		// Attach the texture and the frame buffer
		BindFBO();
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->m_ColorBuffer, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->m_RBO);

		// Verify that the framebuffer is complete
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
		{
			this->m_Logger->Log(LogLevel_Error, "[DISPLAY] : OPENGL Framebuffer is not complete.");
		}

		UnbindFBO();
		UnbindRBO();
	}

	// Binds the display frame buffer object
	OPENVIEWER_FORCEINLINE void Display::BindFBO() const noexcept
	{
		glBindFramebuffer(GL_FRAMEBUFFER, this->m_FBO);
	}

	// Unbinds the display frame buffer object
	OPENVIEWER_FORCEINLINE void Display::UnbindFBO() const noexcept
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// Binds the ddisplay render buffer object
	OPENVIEWER_FORCEINLINE void Display::BindRBO() const noexcept
	{
		glBindRenderbuffer(GL_RENDERBUFFER, this->m_RBO);
	}

	// Unbinds the display render buffer object
	OPENVIEWER_FORCEINLINE void Display::UnbindRBO() const noexcept
	{
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	// Initializes the gl texture that will display the images
	void Display::Initialize(Core::Ocio& ocio) noexcept
	{
		this->m_Logger->Log(LogLevel_Debug, "Initializing display");
		
		const Core::Image* initImage = &this->m_Loader->m_Images[0];
		this->m_Width = initImage->m_Xres;
		this->m_Height = initImage->m_Yres;

		InitializeOpenGL(*initImage);

		// Generate the display texture
		glGenTextures(1, &this->m_DisplayTexture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, this->m_DisplayTexture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		glTexImage2D(GL_TEXTURE_2D, 
					 0, 
					 this->m_Loader->m_Images[0].m_GLInternalFormat, 
					 this->m_Width, 
					 this->m_Height, 
					 0, 
					 this->m_Loader->m_Images[0].m_GLFormat, 
					 this->m_Loader->m_Images[0].m_GLType, 
					 this->m_Loader->m_Cache->m_Items[1].m_Ptr); // The first item starts at 1, index 0 is to signal it is not cached

		// OCIO GPU Processing
		if (ocio.use_gpu > 0)
		{
			auto ocio_start = this->m_Profiler->Start();

			// Bind the framebuffer
			BindFBO();
			glViewport(0, 0, this->m_Width, this->m_Height);
			glEnable(GL_DEPTH_TEST);

			// Clear the framebuffer
			glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Update OCIO Processor and process the image
			ocio.UpdateProcessor();
			ocio.Process(this->m_Width, this->m_Height);

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

			auto ocio_end = this->m_Profiler->End();
			this->m_Profiler->Time("Ocio Transform Time", ocio_start, ocio_end);
		}

		// Unbind our texture
		glBindTexture(GL_TEXTURE_2D, 0);
		
		UnbindFBO();
	}

	// Updates the gl texture that displays the images
	void Display::Update(Core::Ocio& ocio, const uint16_t frameIndex) noexcept
	{
		// Get the different image infos we need to load it
		const uint16_t currentImageXRes = this->m_Loader->m_Images[frameIndex].m_Xres;
		const uint16_t currentImageYRes = this->m_Loader->m_Images[frameIndex].m_Yres;
		const uint64_t currentImageSize = this->m_Loader->m_Images[frameIndex].m_Size;
		const uint16_t currentImageCacheIndex = this->m_Loader->m_Images[frameIndex].m_CacheIndex;
		const void* currentImageCacheAddress = this->m_Loader->m_Cache->m_Items[currentImageCacheIndex].m_Ptr;

		// Error check
		if (currentImageCacheAddress == nullptr)
		{
			StaticDebugConsoleLog("Display Error : Frame Index %d", frameIndex);

			return;
		}

		// Update the texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, this->m_DisplayTexture);
		glTexSubImage2D(GL_TEXTURE_2D,
						0, 0, 0,
						currentImageXRes,
						currentImageYRes,
						this->m_Loader->m_Images[frameIndex].m_GLFormat,
						this->m_Loader->m_Images[frameIndex].m_GLType,
						currentImageCacheAddress);

		// OCIO GPU Processing
		if (ocio.use_gpu > 0)
		{
			auto ocio_start = this->m_Profiler->Start();

			// Bind the framebuffer
			BindFBO();
			glViewport(0, 0, currentImageXRes, currentImageYRes);
			glEnable(GL_DEPTH_TEST);

			// Clear the framebuffer
			glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			ocio.Process(currentImageXRes, currentImageYRes);

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

			auto ocio_end = this->m_Profiler->End();
			this->m_Profiler->Time("Ocio Transform Time", ocio_start, ocio_end);

			// Unbind our texture
			GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
		}
		
	}

	// Main function that contains the window drawing 
	void Display::Draw(uint16_t frameIndex) const noexcept
	{
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoCollapse;

		bool p_open = true;

		ImGui::Begin("Display", &p_open, window_flags);
		{
			ImVec2 size = ImVec2(this->m_Loader->m_Images[frameIndex].m_Xres, 
								 this->m_Loader->m_Images[frameIndex].m_Yres);

			static ImVec2 scrolling;
			static float zoom = 1.0f;
			const ImVec4 tint(1.0f, 1.0f, 1.0f, 1.0f);
			const ImVec4 borderColor(0.0f, 0.0f, 0.0f, 1.0f);

			const bool isHovered = ImGui::IsWindowHovered();

			ImGuiIO& io = ImGui::GetIO();

			if (isHovered && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			{
				scrolling.x += io.MouseDelta.x;
				scrolling.y += io.MouseDelta.y;
			}

			const float mouseWheel = io.MouseWheel;

			if (isHovered && mouseWheel != 0.0f)
			{
				zoom += mouseWheel * 0.1f;
			}

			ImGui::SetCursorPos((ImGui::GetWindowSize() - size * zoom) * 0.5f + scrolling);
			ImGui::Image((void*)(intptr_t)this->m_ColorBuffer, size * zoom, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), tint, borderColor);
		}
		ImGui::End();
	}

	void Display::Release() noexcept
	{
		// Release the loader
		this->m_Loader->Release();
		delete this->m_Loader;

		// delete the gl buffers
		glDeleteFramebuffers(1, &this->m_FBO);
		glDeleteRenderbuffers(1, &this->m_RBO);

		// delete the gl textures
		glDeleteTextures(1, &this->m_DisplayTexture);
		glDeleteTextures(1, &this->m_ColorBuffer);
	}
} // End namespace Interface