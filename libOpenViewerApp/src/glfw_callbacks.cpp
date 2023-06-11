// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewerApp/glfw_callbacks.h"

#define LOV_MEDIAPOOL_SINGLETON
#include "OpenViewer/media_pool.h"

LOVA_NAMESPACE_BEGIN

void glfw_drop_event_callback(GLFWwindow* user_ptr, int count, const char** paths) noexcept
{
    spdlog::debug("[GLFW] : Drop event detected");

    for(size_t i = 0; i < (size_t)count; i++)
    {
        lov::mediapool.add_media(paths[i]);
    }
}

void glfw_key_event_callback(GLFWwindow* user_ptr, int key, int scancode, int action, int mods) noexcept
{
    spdlog::debug("[GLFW] : Key event detected");
}

LOVA_NAMESPACE_END