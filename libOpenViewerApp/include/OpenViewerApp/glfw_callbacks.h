// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#if !defined(__LIBOPENVIEWERAPP_GLFW_CALLBACKS)
#define __LIBOPENVIEWERAPP_GLFW_CALLBACKS

#include "OpenViewerApp/openviewerapp.h"


#include "GLFW/glfw3.h"

LOVA_NAMESPACE_BEGIN

static void glfw_error_callback(int error, const char* desc) noexcept
{
    spdlog::error("[GLFW] : {} ({})", error, desc);
}

void glfw_drop_event_callback(GLFWwindow* user_ptr, int count, const char** paths) noexcept;

void glfw_key_event_callback(GLFWwindow* user_ptr, int key, int scancode, int action, int mods) noexcept;

LOVA_NAMESPACE_END

#endif // !defined(__LIBOPENVIEWERAPP_GLFW_CALLBACKS)