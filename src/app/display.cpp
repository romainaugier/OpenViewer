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
		glGenTextures(1, &this->m_TransformedTexture);
		glBindTexture(GL_TEXTURE_2D, this->m_TransformedTexture);
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
		this->BindRBO();
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, this->m_Width, this->m_Height);

		// Attach the texture and the frame buffer
		this->BindFBO();
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->m_TransformedTexture, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->m_RBO);

		// Verify that the framebuffer is complete
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
		{
			this->m_Logger->Log(LogLevel_Error, "[DISPLAY] : OPENGL Framebuffer is not complete.");
		}

		this->UnbindFBO();
		this->UnbindRBO();
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
		this->m_Logger->Log(LogLevel_Diagnostic, "[DISPLAY] : Initializing display %d", this->m_DisplayID);
		
		// When we initialize a display, we have at least one media and one image loader
		const Core::Image* initImage = &this->m_Loader->m_Medias[0].m_Images[0];
		this->m_Width = initImage->m_Xres;
		this->m_Height = initImage->m_Yres;

		InitializeOpenGL(*initImage);

		// Generate the display texture
		glGenTextures(1, &this->m_RawTexture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, this->m_RawTexture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		glTexImage2D(GL_TEXTURE_2D, 
					 0, 
					 initImage->m_GLInternalFormat, 
					 this->m_Width, 
					 this->m_Height, 
					 0, 
					 initImage->m_GLFormat, 
					 initImage->m_GLType, 
					 this->m_Loader->m_Cache->m_Items[1].m_DataPtr); // The first item starts at 1, index 0 is to signal it is not cached

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

	void Display::ReInitialize(const Core::Image& image, Core::Ocio& ocio) noexcept
	{
		this->m_Logger->Log(LogLevel_Diagnostic, "[DISPLAY] : Image dimensions changed, reinitializing display %d", this->m_DisplayID);
		
		this->m_Width = image.m_Xres;
		this->m_Height = image.m_Yres;

		InitializeOpenGL(image);

		// Generate the display texture
		glGenTextures(1, &this->m_RawTexture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, this->m_RawTexture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		glTexImage2D(GL_TEXTURE_2D, 
					 0, 
					 image.m_GLInternalFormat, 
					 this->m_Width, 
					 this->m_Height, 
					 0, 
					 image.m_GLFormat, 
					 image.m_GLType, 
					 this->m_Loader->m_Cache->m_Items[image.m_CacheIndex].m_DataPtr); // The first item starts at 1, index 0 is to signal it is not cached

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
	void Display::Update(Core::Ocio& ocio, const uint32_t frameIndex) noexcept
	{
		// Get the image to be displayed
		auto getStart = this->m_Profiler->Start();
		Core::Image* currentImage = this->m_Loader->GetImage(frameIndex);
		auto getEnd = this->m_Profiler->End();
		this->m_Profiler->Time("Display Image Get Time", getStart, getEnd);

		if (currentImage != nullptr && currentImage->m_CacheIndex > 0)
		{
			if (currentImage->m_Xres != this->m_Width || currentImage->m_Yres != this->m_Height)
			{
				this->ReInitialize(*currentImage, ocio);
			}
			else
			{
				// Get the different image infos we need to load it
				const auto getImgInfoStart = this->m_Profiler->Start();
				const uint16_t currentImageXRes = currentImage->m_Xres;
				const uint16_t currentImageYRes = currentImage->m_Yres;
				const uint64_t currentImageSize = currentImage->m_Size;
				const void* currentImageCacheAddress = this->m_Loader->m_Cache->m_Items[currentImage->m_CacheIndex].m_DataPtr;
				const auto getImgInfoEnd = this->m_Profiler->End();
				this->m_Profiler->Time("Display Image Infos Time", getImgInfoStart, getImgInfoEnd);

				// Update the texture
				const auto texUpdateStart = this->m_Profiler->Start();

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, this->m_RawTexture);
				glTexImage2D(GL_TEXTURE_2D, 
							 0, 
							 currentImage->m_GLInternalFormat, 
							 currentImageXRes, 
							 currentImageYRes, 
							 0, 
							 currentImage->m_GLFormat, 
							 currentImage->m_GLType, 
							 currentImageCacheAddress);

				const auto texUpdateEnd = this->m_Profiler->End();
				this->m_Profiler->Time("Display Texture Update Time", texUpdateStart, texUpdateEnd);

				// OCIO GPU Processing
				if (ocio.use_gpu > 0)
				{
					const auto ocio_start = this->m_Profiler->Start();

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

					const auto ocio_end = this->m_Profiler->End();
					this->m_Profiler->Time("Ocio Transform Time", ocio_start, ocio_end);

					// Unbind our texture
					glBindTexture(GL_TEXTURE_2D, 0);
				}
			}
		}
		else
		{
			this->m_Logger->Log(LogLevel_Debug, "[DISPLAY] : Skipping to next frame.");
		}
	}

	// Main function that contains the window drawing 
	void Display::Draw(uint32_t frameIndex) noexcept
	{
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

		bool p_open = true;

		char displayName[64];
		Utils::Str::Format(displayName, "Display %d", this->m_DisplayID);

		ImGui::Begin(displayName, &p_open, window_flags);
		{
			const ImVec2 size = ImVec2(this->m_Width, 
								       this->m_Height);

			static ImVec2 scrolling;
			static float zoom = 1.0f;
			const ImVec4 tint(1.0f, 1.0f, 1.0f, 1.0f);
			const ImVec4 borderColor(0.0f, 0.0f, 0.0f, 1.0f);

			const bool isHovered = ImGui::IsWindowHovered();
			const bool isFocused = ImGui::IsWindowFocused();
			const bool isActive = isFocused;

			ImGuiIO& io = ImGui::GetIO();

			if (isActive && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			{
				scrolling.x += io.MouseDelta.x;
				scrolling.y += io.MouseDelta.y;

				this->m_DisplayPos += io.MouseDelta;
			}

			const float mouseWheel = io.MouseWheel;
			bool hasZoomed = false;
			

			if (isActive && mouseWheel != 0.0f)
			{
				ImVec2 mouseLocal = (ImGui::GetMousePos() - this->m_DisplayPos) / zoom;
				
				if (mouseWheel > 0.0f) zoom *= 1.1f;
				else zoom /= 1.1f;

				zoom = zoom < 0.01f ? 0.01f : zoom;
				
				mouseLocal = this->m_DisplayPos + mouseLocal * zoom;
				
				this->m_DisplayPos += ImGui::GetMousePos() - mouseLocal;
			}

			const ImVec2 zoomedSize = size * zoom;

			ImGui::SetCursorScreenPos(this->m_DisplayPos);

			const ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();

			ImGui::Image((void*)(intptr_t)this->m_TransformedTexture, zoomedSize, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), tint, borderColor);

			if (ImGui::IsItemHovered())
			{
				this->m_IsImageHovered = true;
				this->m_HoverCoordinates = ImClamp(ImGui::Fit(ImGui::GetMousePos() - cursorScreenPos, 
												              ImVec2(0, 0),
												              size * zoom,
												              ImVec2(0, 0),
												              size), ImVec2(0, 0), size);
			}                 
			else
			{
				this->m_IsImageHovered = false;
			}
		}
		ImGui::End();
	}

	ImVec4 Display::GetPixel(const uint16_t x, const uint16_t y) const noexcept
	{
		ImVec4 color;
		
		this->BindFBO();

		glBindTexture(GL_TEXTURE_2D, this->m_TransformedTexture);

		glReadPixels(x, y, 1, 1, GL_RGBA, GL_FLOAT, static_cast<void*>(&color));

		glBindTexture(GL_TEXTURE_2D, 0);

		this->UnbindFBO();

		return color;
	}

	void Display::Release() noexcept
	{
		// delete the gl buffers
		glDeleteFramebuffers(1, &this->m_FBO);
		glDeleteRenderbuffers(1, &this->m_RBO);

		// delete the gl textures
		glDeleteTextures(1, &this->m_RawTexture);
		glDeleteTextures(1, &this->m_TransformedTexture);

		this->m_Logger->Log(LogLevel_Diagnostic, "[DISPLAY] : Released display %d", this->m_DisplayID);
	}
} // End namespace Interface