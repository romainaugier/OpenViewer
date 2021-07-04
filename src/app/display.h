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

#include "core/loader.h"
#include "core/ocio.h"
#include "utils/profiler.h"

struct Display
{
	float* buffer = nullptr;
	GLuint display_tex;
	GLuint tex_color_buffer;
	GLuint fbo, rbo;
	unsigned int display : 1;
	unsigned int use_buffer : 1;

	Display()
	{
		display = 0;
		use_buffer = 0;
	}

	void Initialize(const Loader& loader, Ocio& ocio, Profiler& prof) noexcept;
	void InitializeOpenGL(const uint16_t width, const uint16_t height) noexcept;
	__forceinline void BindFBO() const noexcept;
	__forceinline void UnbindFBO() const noexcept;
	__forceinline void BindRBO() const noexcept;
	__forceinline void UnbindRBO() const noexcept;
	void Update(const Loader& loader, Ocio& ocio, const uint16_t frame_idx, Profiler& prof) noexcept;
	void Draw(Loader& loader, uint16_t frame_idx) const noexcept;
	void __vectorcall ToFloat(const half* __restrict half_buffer, const int64_t size) noexcept;
	void Release() noexcept;
};