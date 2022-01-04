// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#pragma once

#include <stdint.h>

#include "gl/glew.h"

#include "utils/decl.h"
#include "utils/logger.h"

namespace Interface
{
	namespace Plot
	{
		struct Parade
		{
			GLuint m_RenderTexture;
			GLuint m_ImageTexture;
			GLuint m_RBO;
			GLuint m_FBO;
			
			uint16_t m_Width = 1000;
			uint16_t m_Height = 500;

			void Initialize() noexcept;

			void Update(const GLuint imageTextureID) noexcept;

			OPENVIEWER_FORCEINLINE void BindFBO() const noexcept;
			OPENVIEWER_FORCEINLINE void UnbindFBO() const noexcept;
			OPENVIEWER_FORCEINLINE void BindRBO() const noexcept;
			OPENVIEWER_FORCEINLINE void UnbindRBO() const noexcept;
		};

		struct Waveform
		{

		};
	}
}