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

		// Get the alpha blending shader
		const std::string vertShaderPath = Utils::Fs::ExpandCwd("/shaders/alpha_blend.vert");
		const std::string fragShaderPath = Utils::Fs::ExpandCwd("/shaders/alpha_blend.frag");

		this->m_AlphaBlendingShader.LoadAndCompile(vertShaderPath.c_str(), fragShaderPath.c_str());
	}

	void Display::InitAlphaBlendingTexture() noexcept
	{
		// Generate the image to make the alpha blending
		glGenTextures(1, &this->m_DisplayTexture);
		glBindTexture(GL_TEXTURE_2D, this->m_DisplayTexture);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, this->m_Width, this->m_Height);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Display::InitRawTexture(const Core::Image* initImage) noexcept
	{
		// Generate the texture that reads the pixels from the image
		// They are raw, meaning no transformation has been applied yet
		glGenTextures(1, &this->m_RawTexture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, this->m_RawTexture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 
					 0, 
					 initImage->m_GLInternalFormat, 
					 this->m_Width, 
					 this->m_Height, 
					 0, 
					 initImage->m_GLFormat, 
					 initImage->m_GLType, 
					 this->m_Loader->m_Cache->m_Items[1].m_DataPtr); // The first item starts at 1, index 0 is to signal it is not cached

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Display::OcioTransform(Core::Ocio& ocio, const bool updateProcessor) noexcept
	{
		// Bind the raw texture
		glBindTexture(GL_TEXTURE_2D, this->m_RawTexture);

		auto ocio_start = this->m_Profiler->Start();

		// Bind the framebuffer
		BindFBO();
		glViewport(0, 0, this->m_Width, this->m_Height);
		glEnable(GL_DEPTH_TEST);

		// Clear the framebuffer
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Update OCIO Processor and process the image
		if (updateProcessor) ocio.UpdateProcessor();
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

		// Unbind our texture
		glBindTexture(GL_TEXTURE_2D, 0);

		auto ocio_end = this->m_Profiler->End();
		this->m_Profiler->Time("Ocio Transform Time", ocio_start, ocio_end);
	}

	void Display::AlphaBlending() noexcept
	{
		// Bind the image texture at binding point 1
		glBindImageTexture(1, this->m_DisplayTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);	

		this->m_AlphaBlendingShader.Use();
		this->m_AlphaBlendingShader.SetInt("mode", this->m_BackGroundMode);
		this->m_AlphaBlendingShader.SetInt("width", this->m_Width);
		this->m_AlphaBlendingShader.SetInt("height", this->m_Height);
		this->m_AlphaBlendingShader.SetInt("premultiply", this->m_PremultiplyAlpha ? 1 : 0);

		// Bind the ocio transformed texture
		glActiveTexture(GL_TEXTURE0);

		glBindTexture(GL_TEXTURE_2D, this->m_TransformedTexture);

		// Attribute-less shader rendering, we launch a thread for each pixel
		// by drawing an array of width * height vertices
		glDrawArrays(GL_POINTS, 0, this->m_Width * this->m_Height);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// Initializes the gl texture that will display the images
	void Display::Initialize(Core::Ocio& ocio, const uint32_t mediaId) noexcept
	{
		this->m_Logger->Log(LogLevel_Diagnostic, "[DISPLAY] : Initializing display %d", this->m_DisplayID);

		this->m_MediaID = mediaId;
		
		// When we initialize a display, we have at least one media and one image loader
		const Core::Image* initImage = &this->m_Loader->m_Medias[0].m_Images[0];
		this->m_Width = initImage->m_Xres;
		this->m_Height = initImage->m_Yres;

		// Checkerboard by default
		this->m_BackGroundMode = 2;

		InitializeOpenGL(*initImage);

		this->InitAlphaBlendingTexture();
		this->InitRawTexture(initImage);

		// OCIO Transform and processor update
		this->OcioTransform(ocio);

		// Now that we have the ocio transformed texture, make a correct alpha blending
		this->AlphaBlending();
	}

	void Display::ReInitialize(const Core::Image& image, Core::Ocio& ocio, const uint32_t mediaId) noexcept
	{
		this->m_Logger->Log(LogLevel_Diagnostic, "[DISPLAY] : Image dimensions changed, reinitializing display %d", this->m_DisplayID);
		
		this->m_MediaID = mediaId;

		this->m_Width = image.m_Xres;
		this->m_Height = image.m_Yres;

		// Delete all the textures
		glDeleteTextures(1, &this->m_RawTexture);
		glDeleteTextures(1, &this->m_TransformedTexture);
		glDeleteTextures(1, &this->m_DisplayTexture);

		InitializeOpenGL(image);

		this->InitAlphaBlendingTexture();
		this->InitRawTexture(&image);

		// OCIO Transform and processor update
		this->OcioTransform(ocio);

		// Now that we have the ocio transformed texture, make a correct alpha blending
		this->AlphaBlending();
	}

	// Updates the gl texture that displays the images
	void Display::Update(Core::Ocio& ocio, const uint32_t frameIndex) noexcept
	{
		// Get the image to be displayed
		Core::Image* currentImage = this->m_Loader->GetImage(frameIndex);

		if (currentImage != nullptr && currentImage->m_CacheIndex > 0)
		{
			if (this->m_Width != currentImage->m_Xres && this->m_Height != currentImage->m_Yres || this->m_NeedReinitialization)
			{
				this->ReInitialize(*currentImage, ocio, this->m_MediaID);

				this->NeedReinit(false);

				return;
			}

			// Update the texture
			const auto texUpdateStart = this->m_Profiler->Start();
			
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, this->m_RawTexture);
			glTexImage2D(GL_TEXTURE_2D, 
						 0, 
						 currentImage->m_GLInternalFormat, 
						 currentImage->m_Xres,
						 currentImage->m_Yres, 
						 0, 
						 currentImage->m_GLFormat, 
						 currentImage->m_GLType, 
						 this->m_Loader->GetImageCachePtrFromIndex(currentImage->m_CacheIndex));
			glBindTexture(GL_TEXTURE_2D, 0);

			const auto texUpdateEnd = this->m_Profiler->End();
			this->m_Profiler->Time("Display Texture Update Time", texUpdateStart, texUpdateEnd);

			this->OcioTransform(ocio, false);

			// Now that we have the ocio transformed texture, make a correct alpha blending
			this->AlphaBlending();
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

		char displayName[64];
		Utils::Str::Format(displayName, "Display %d", this->m_DisplayID);

		if (this->m_IsOpen)
		{
			ImGui::Begin(displayName, &this->m_IsOpen, window_flags);
			{
				if (ImGui::IsWindowFocused()) this->m_IsActive = true;
				// else this->m_IsActive = false;

				const ImVec2 size = ImVec2(this->m_Width, 
										   this->m_Height);

				ImVec2 uv0 = ImVec2(0.0f, 0.0f);
				ImVec2 uv1 = ImVec2(1.0f, 1.0f);

				if (this->m_HorizontalMirrorView)
				{
					uv0.x = 1.0f;
					uv1.x = 0.0f;
				}
				if (this->m_VerticalMirrorView)
				{
					uv0.y = 1.0f;
					uv1.y = 0.0f;
				}

				static ImVec2 scrolling;
				static float zoom = 1.0f;
				const ImVec4 tint(1.0f, 1.0f, 1.0f, 1.0f);
				const ImVec4 borderColor(0.0f, 0.0f, 0.0f, 1.0f);

				const bool isHovered = ImGui::IsWindowHovered();
				const bool isFocused = ImGui::IsWindowFocused();
				const bool isActive = isFocused;

				ImGuiIO& io = ImGui::GetIO();

				if ((isActive || isHovered) && ImGui::IsMouseDragging(ImGuiMouseButton_Right))
				{
					scrolling.x += io.MouseDelta.x;
					scrolling.y += io.MouseDelta.y;

					this->m_DisplayPos += io.MouseDelta;
				}

				const float mouseWheel = io.MouseWheel;
				bool hasZoomed = false;

				if ((isActive || isHovered) && mouseWheel != 0.0f)
				{
					ImVec2 mouseLocal = (ImGui::GetMousePos() - this->m_DisplayPos) / zoom;
					
					if (mouseWheel > 0.0f) zoom *= 1.1f;
					else zoom /= 1.1f;

					zoom = zoom < 0.01f ? 0.01f : zoom;
					
					mouseLocal = this->m_DisplayPos + mouseLocal * zoom;
					
					this->m_DisplayPos += ImGui::GetMousePos() - mouseLocal;
				}

				if (this->m_HomeView)
				{
					const ImVec2 windowCenter = (ImGui::GetWindowPos() + ImGui::GetWindowSize()) / 2.0f;
					this->m_DisplayPos = windowCenter - (size / 2.0f);
					zoom = 1.0f;
				}
				else if (this->m_FrameView)
				{
					const ImVec2 windowCenter = (ImGui::GetWindowPos() + ImGui::GetWindowSize()) / 2.0f;
					const float xRatio = ImGui::GetWindowSize().x / size.x; 
					const float yRatio = ImGui::GetWindowSize().y / size.y;
					zoom = xRatio < yRatio ? xRatio : yRatio; 
					
					this->m_DisplayPos = windowCenter - ((size * zoom) / 2.0f);
				}

				const ImVec2 zoomedSize = size * zoom;

				ImGui::SetCursorScreenPos(this->m_DisplayPos);

				const ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();

				ImDrawList* drawlist = ImGui::GetWindowDrawList();

				ImGui::Image((void*)(intptr_t)this->m_DisplayTexture, zoomedSize, uv0, uv1, tint, borderColor);

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

		this->m_FrameView = false;
		this->m_HomeView = false;
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
		glDeleteTextures(1, &this->m_DisplayTexture);

		this->m_Logger->Log(LogLevel_Diagnostic, "[DISPLAY] : Released display %d", this->m_DisplayID);
	}
} // End namespace Interface