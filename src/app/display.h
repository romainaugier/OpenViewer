// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#pragma once

#include "imgui.h"

#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui_internal.h"

#include "GL/gl3w.h"
#include "GLFW/glfw3.h"
#include "half.h"
#include <immintrin.h>

#include "core/loader.h"
#include "core/ocio.h"

struct Display
{
	float* buffer = nullptr;
	GLuint display_tex;
	unsigned int display : 1;
	unsigned int use_buffer : 1;

	Display()
	{
		display = 0;
		use_buffer = 0;
	}

	void Initialize(const Loader& loader, Ocio& ocio) noexcept;
	void Update(const Loader& loader, Ocio& ocio, const uint16_t frame_idx) noexcept;
	void Draw(Loader& loader, uint16_t frame_idx) const noexcept;
	void __vectorcall ToFloat(const half* __restrict half_buffer, const int64_t size) noexcept;
	void Release() noexcept;
};