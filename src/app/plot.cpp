// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "plot.h"

namespace Interface
{
	namespace Plot
	{
		void Parade::Initialize() noexcept
		{
			// Load and compile shaders
			std::string currentPath = std::filesystem::current_path().string();

			Utils::Str::CleanOSPath(currentPath);

			char vertexShaderPath[2048];
			Utils::Str::Format(vertexShaderPath, "%s/shaders/common.vert", currentPath.c_str());
			
			char fragmentShaderPath[2048];
			Utils::Str::Format(fragmentShaderPath, "%s/shaders/parade.frag", currentPath.c_str());

			this->m_Shader.LoadAndCompile(vertexShaderPath, fragmentShaderPath);

			// Generate the image for load/store
			glGenTextures(1, &this->m_DrawTexture);
			glBindTexture(GL_TEXTURE_2D, this->m_DrawTexture);
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8UI, this->m_Width, this->m_Height);
			glBindTexture(GL_TEXTURE_2D, 0);

			// Generate the frame buffer object
			glGenFramebuffers(1, &this->m_FBO);

			// Generate the color attachement texture
			glGenTextures(1, &this->m_RenderTexture);
			glBindTexture(GL_TEXTURE_2D, this->m_RenderTexture);
			glTexImage2D(GL_TEXTURE_2D, 
						 0, 
						 GL_RGB8,
						 this->m_Width, 
						 this->m_Height, 
						 0, 
						 GL_RGB, 
						 GL_UNSIGNED_BYTE, 
						 nullptr);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			// Generate the render buffer object
			glGenRenderbuffers(1, &this->m_RBO);
			glBindRenderbuffer(GL_RENDERBUFFER, this->m_RBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, this->m_Width, this->m_Height);

			// Attach the texture and the frame buffer
			glBindFramebuffer(GL_FRAMEBUFFER, this->m_FBO);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->m_RenderTexture, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->m_RBO);

			// Verify that the framebuffer is complete
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
			{
				StaticErrorConsoleLog("[DISPLAY] : OPENGL Framebuffer is not complete.");
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			glBindTexture(GL_TEXTURE_2D, 0);

			// Generate the texture to process in the framebuffer
			glGenTextures(1, &this->m_DrawTexture);
			glBindTexture(GL_TEXTURE_2D, this->m_DrawTexture);
			glTexImage2D(GL_TEXTURE_2D, 
						 0, 
						 GL_RGB8,
						 this->m_Width, 
						 this->m_Height, 
						 0, 
						 GL_RGB, 
						 GL_UNSIGNED_BYTE, 
						 nullptr);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glBindTexture(GL_TEXTURE_2D, 0);

			// Generate VBO
			float vertices[] = {
				1.0f,  1.0f, 0.0f,   1.0f, 1.0f,
				1.0f, -1.0f, 0.0f,   1.0f, 0.0f,
				-1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
				-1.0f,  1.0f, 0.0f,  0.0f, 1.0f 
			};

			unsigned int indices[] = {
				0, 1, 3,
				1, 2, 3
			};

			glGenVertexArrays(1, &this->m_VAO);
			glGenBuffers(1, &this->m_VBO);
			glGenBuffers(1, &this->m_EBO);

			glBindVertexArray(this->m_VAO);

			glBindBuffer(GL_ARRAY_BUFFER, this->m_VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);
		}

		void Parade::Update(const GLuint imageTextureID) noexcept
		{
			// Bind the image texture at binding point 3
			glBindImageTexture(0, this->m_DrawTexture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8UI);

			// Bind the framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, this->m_FBO); 
			glViewport(0, 0, this->m_Width, this->m_Height);
			glDisable(GL_DEPTH_TEST);

			// Clear the framebuffer
			glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Draw the quad
			//glEnable(GL_TEXTURE_2D);

			this->m_Shader.Use();

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, imageTextureID);
			glBindVertexArray(this->m_VAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			glBindVertexArray(0);
			glBindTexture(GL_TEXTURE_2D, 0);

			//glDisable(GL_TEXTURE_2D);

			// Unbind frame buffer object and clear
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		void Parade::Draw() const noexcept
		{
			if (this->m_ShowWindow)
			{
				ImGui::Begin("RGB Parade", (bool*)&this->m_ShowWindow);
				{
					const ImVec4 tint(1.0f, 1.0f, 1.0f, 1.0f);
					const ImVec4 borderColor(0.0f, 0.0f, 0.0f, 1.0f);
					const ImVec2 size = ImVec2(this->m_Width, this->m_Height);

					ImGui::Image((void*)(intptr_t)this->m_DrawTexture, size, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), tint, borderColor);
				}
				ImGui::End();
			}
		}

		void Parade::Release() noexcept
		{
			glDeleteVertexArrays(1, &this->m_VAO);
			glDeleteBuffers(1, &this->m_VBO);
			glDeleteBuffers(1, &this->m_EBO);
		}
	}
}