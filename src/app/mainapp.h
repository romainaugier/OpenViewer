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

#include "imginfos.h"
#include "mediaexplorer.h"
#include "core/loader.h"
#include "core/parser.h"
#include "style.h"

int application(int argc, char** argv);

// Glfw callbacks

OV_STATIC_FUNC void GLFWErrorCallback(int error, const char* description)
{
    StaticErrorConsoleLog("[GLFW] : Code : %d : %s.", error, description);
}

OV_STATIC_FUNC void GLFWDropEventCallback(GLFWwindow* window, int count, const char** paths)
{
    Interface::Application* app = static_cast<Interface::Application*>(glfwGetWindowUserPointer(window));

    app->m_Logger->Log(LogLevel_Debug, "[MAIN] : Drop event detected");
    app->m_Playbar->GoFirstFrame();

    const uint32_t mediaCount = app->m_Loader->GetMediaCount();

    for(uint32_t i = 0; i < count; i++)
    {
        app->m_Loader->Load(paths[i]);
    }

    // If no display is active, create one
    if (app->m_DisplayCount == 0)
    {
        app->m_Loader->m_Cache->Flush();
        
        app->m_Loader->SetAllMediasInactive();

        app->m_Loader->SetMediaActive(mediaCount);

        app->m_Loader->LoadImageToCache(0);
        
        Interface::Display* newDisplay = new Interface::Display(app->m_Loader->m_Profiler, app->m_Logger, app->m_Loader, 1);

        newDisplay->Initialize(*app->m_OcioModule, mediaCount);
        newDisplay->NeedFrame();
        
        app->m_Displays[++app->m_DisplayCount] = std::make_pair(true, newDisplay);
        app->m_ActiveDisplayID = 1;
    }
    else
    {
        app->m_Loader->m_Cache->Flush();

        app->m_Loader->SetAllMediasInactive();

        app->m_Loader->SetMediaActive(mediaCount);

        Interface::Display* activeDisplay = app->GetActiveDisplay();
        activeDisplay->NeedReinit();
        activeDisplay->NeedFrame();
        activeDisplay->m_MediaID = mediaCount;

        app->m_Loader->LoadImageToCache(0);

        app->Changed();
    }
    
    app->m_Playbar->SetRange(app->m_Loader->m_Medias[mediaCount].m_TimelineRange);
}

OV_STATIC_FUNC void GLFWKeyEventCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Interface::Application* app = static_cast<Interface::Application*>(glfwGetWindowUserPointer(window));

    if (action == GLFW_PRESS || action == GLFW_RELEASE) 
    {
        app->m_Shortcuts.Keys(window, key, scancode, action, mods);
        app->HandleShortcuts();
    }
}