// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "GL/glew.h"

#include "OpenViewerApp/app.h" 
#include "OpenViewerApp/glfw_callbacks.h"
#include "OpenViewerApp/media_pool.h"
#include "OpenViewerApp/timeline.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include "OpenColorIO/OpenColorABI.h"
#include "OpenImageIO/oiioversion.h"

LOVA_NAMESPACE_BEGIN

LOVA_API int app(int argc, char** argv) noexcept
{
    spdlog::info("OpenViewer App");
    spdlog::info("ImGui Version : {}", IMGUI_VERSION);
    spdlog::info("OCIO Version : {}", OCIO_VERSION);
    spdlog::info("OIIO Version : {}", OIIO_VERSION_STRING);

    glfwSetErrorCallback(glfw_error_callback);

    if(!glfwInit())
    {
        spdlog::error("[GLFW] : Failed to initialize glfw");
        return 1;
    }

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "OpenViewer", nullptr, nullptr);

    if(window == nullptr)
    {
        spdlog::error("[GLFW] : Failed to create window. Exiting application");
        glfwTerminate();
        return 1;
    }

    glfwSetDropCallback(window, glfw_drop_event_callback);
    glfwSetKeyCallback(window, glfw_key_event_callback);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    GLenum err = glewInit();

    if(err != GLEW_OK)
    {
        spdlog::error("[GLEW] : Failed to initialize glew : {}", glewGetErrorString(err));
        glfwTerminate();
        return 1;
    }


    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // OpenViewer widgets

    MediaPoolWidget media_pool_widget;
    TimelineWidget timeline_widget;

    timeline_widget.internal_timeline.set_global_range(1, 400);
    timeline_widget.internal_timeline.set_focus_range(100, 300);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Draw OpenViewer widgets
        media_pool_widget.draw();
        timeline_widget.draw();


        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

LOVA_NAMESPACE_END
