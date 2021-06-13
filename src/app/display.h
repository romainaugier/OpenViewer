// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#pragma once

#include "imgui.h"

#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui_internal.h"

#include "GL/gl3w.h"
#include "GLFW/glfw3.h"

#include "core/loader.h"

struct Display
{
	void* buffer;
	GLuint display_tex;
	unsigned int display : 1;

	Display()
	{
		display = 0;
	}

	void init(Loader& loader) noexcept;
	void update(Loader& loader, uint16_t frame_idx) noexcept;
	void draw(Loader& loader, uint16_t frame_idx) noexcept;
};