// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include "imgui.h"

#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui_internal.h"

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "utils/gl_utils.h"

#include "half.h"
#include <immintrin.h>

#include "plot.h"
#include "core/loader.h"
#include "core/ocio.h"
#include "utils/profiler.h"
#include "utils/memory/alloc.h"

namespace Interface
{
	struct Display
	{
		Profiler* m_Profiler;

		Logger* m_Logger;
		
		Core::Loader* m_Loader;

		GLuint m_DisplayTexture;
		GLuint m_ColorBuffer;
		GLuint m_FBO, m_RBO;
		
		uint16_t m_Width;
		uint16_t m_Height;

		uint8_t m_MipMapIndex;
		uint8_t m_DisplayID = 0;

		bool m_IsOpen = true;
		bool m_IsActive = false;

		Display(Profiler* profiler, Logger* logger, const uint8_t id)
		{
			this->m_Profiler = profiler;
			this->m_Logger = logger;
			this->m_Loader = new Core::Loader(logger, profiler);
			this->m_DisplayID = id;
		}

		void Initialize(Core::Ocio& ocio) noexcept;
		void InitializeOpenGL(const Core::Image& image) noexcept;
		OPENVIEWER_FORCEINLINE void BindFBO() const noexcept;
		OPENVIEWER_FORCEINLINE void UnbindFBO() const noexcept;
		OPENVIEWER_FORCEINLINE void BindRBO() const noexcept;
		OPENVIEWER_FORCEINLINE void UnbindRBO() const noexcept;
		void Update(Core::Ocio& ocio, const uint16_t frameIndex) noexcept;
		void Draw(uint16_t frameIndex) const noexcept;
		void Release() noexcept;
	};
} // End namespace Interface