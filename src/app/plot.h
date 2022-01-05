// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#pragma once

#include <stdint.h>

#include "gl/glew.h"
#include "implot.h"
#include "imgui.h"

#include "utils/decl.h"
#include "utils/gl_utils.h"
#include "utils/filesystem_utils.h"

namespace Interface
{
	namespace Plot
	{
		struct Parade
		{
			Utils::GL::Shader m_Shader;

			GLuint m_RenderTexture;
			GLuint m_DrawTexture;
			GLuint m_RBO;
			GLuint m_FBO;
			GLuint m_VAO;
			GLuint m_VBO;
			GLuint m_EBO;
			
			uint16_t m_Width = 1000;
			uint16_t m_Height = 500;

			bool m_ShowWindow = true;

			void Initialize() noexcept;

			void Update(const GLuint imageTextureID) noexcept;

			void Draw() const noexcept;

			void Release() noexcept;
		};

		struct Waveform
		{

		};
	}
}