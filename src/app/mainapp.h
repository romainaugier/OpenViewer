// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>

#include <GL/glew.h>

#include <GLFW/glfw3.h>


#if defined(OPENVIEWER_MSVC) && (OPENVIEWER_MSVC >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

#include <thread>

#include "menubar.h"
#include "imginfos.h"
#include "mediaexplorer.h"
#include "core/loader.h"
#include "core/parser.h"

int application(int argc, char** argv);

// Glfw callbacks

OPENVIEWER_STATIC_FUNC void GLFWErrorCallback(int error, const char* description)
{
    StaticErrorConsoleLog("[GLFW] : Code : %d : %s. Exiting application", error, description);

    std::exit(EXIT_FAILURE);
}

OPENVIEWER_STATIC_FUNC void GLFWDropEventCallback(GLFWwindow* window, int count, const char** paths)
{
    Interface::Application* app = static_cast<Interface::Application*>(glfwGetWindowUserPointer(window));

    app->m_Logger->Log(LogLevel_Debug, "[MAIN] : Drop event detected");

    for(uint32_t i = 0; i < count; i++)
    {
        app->m_Loader->Load(paths[i]);
    }
}

OPENVIEWER_STATIC_FUNC void GLFWKeyEventCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Interface::Application* app = static_cast<Interface::Application*>(glfwGetWindowUserPointer(window));
    app->m_Shortcuts.Keys(window, key, scancode, action, mods);
}