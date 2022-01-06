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
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, this->m_Width, this->m_Height);
			glBindTexture(GL_TEXTURE_2D, 0);

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
			glClear(GL_COLOR_BUFFER_BIT);

			// Bind the image texture at binding point 1
			glBindImageTexture(1, this->m_DrawTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

			glBindVertexArray(this->m_VAO);

			this->m_Shader.Use();

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, imageTextureID);
			
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			glBindVertexArray(0);

			glBindTexture(GL_TEXTURE_2D, 0);
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