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

struct Display
{
	float* buffer = nullptr;
	uint32_t buffer_size = 0;
	GLuint display_tex;
	GLuint tex_color_buffer;
	GLuint fbo, rbo;
	Profiler* profiler;
	uint16_t width;
	uint16_t height;
	uint8_t mipmap_idx;
	unsigned int display : 1;
	unsigned int use_buffer : 1;

	Display(Profiler* prof)
	{
		profiler = prof;
		display = 0;
		use_buffer = 0;
	}

	void Initialize(const Loader& loader, Ocio& ocio) noexcept;
	void InitializeOpenGL(const Image& img) noexcept;
	OPENVIEWER_FORCEINLINE void BindFBO() const noexcept;
	OPENVIEWER_FORCEINLINE void UnbindFBO() const noexcept;
	OPENVIEWER_FORCEINLINE void BindRBO() const noexcept;
	OPENVIEWER_FORCEINLINE void UnbindRBO() const noexcept;
	void Update(const Loader& loader, Ocio& ocio, const uint16_t frame_idx) noexcept;
	void Draw(Loader& loader, uint16_t frame_idx) const noexcept;
	void OPENVIEWER_VECTORCALL Unpack(const half* __restrict half_buffer, const int64_t size, bool add_alpha) noexcept;
	void GetDisplayPixels() noexcept;
	void Release() noexcept;
};