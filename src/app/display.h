#pragma once

#include "imgui.h"

#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui_internal.h"

#include "GLFW/glfw3.h"

struct Display
{
	void draw(GLuint tex_id, ImVec2 size);
};