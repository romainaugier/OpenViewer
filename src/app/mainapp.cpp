// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "mainapp.h"

int application(int argc, char** argv)
{
    // Initialize the application
    
    // Logger
    Logger logger;
    logger.SetLevel(LogLevel_Diagnostic);
    logger.Log(LogLevel_Diagnostic, "[MAIN] : Initializing OpenViewer");
    
    // Profiler
    Profiler profiler;
    
    // Ocio 
    Core::Ocio ocio(&logger);
    ocio.Initialize();

    // Loader/Cache
    Core::Loader loader(&logger, &profiler);

    Interface::Application app(&logger, &loader, &ocio);

    // Setup GLFW
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit())
    {
        logger.Log(LogLevel_Error, "[GLFW] : Failed to initialize GLFW. Exiting application");
        return 1;
    }

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "OpenViewer", NULL, NULL);
    if (window == NULL)
        return 1;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
    bool err = glewInit() != 0;

    if (err)
    {
        logger.Log(LogLevel_Error, "[OPENGL] : Failed to initialize loader. Exiting application");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Docking
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    
    // MultiViewport
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // Style
    ImGuiStyle* style = &ImGui::GetStyle();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    GL_CHECK(ImGui_ImplOpenGL3_Init(glsl_version));

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);

    // Initialize parser to get command line arguments, if any
    Parser parser(argc, argv);

    // Initialize the different windows
    Interface::Settings_Windows settings;
    settings.GetOcioConfig(ocio);

    Interface::ImageInfo imageInfosWindow;
    Interface::PixelInfo pixelInfosWindow;
    Interface::MediaExplorer mediaExplorerWindow(&loader, &logger);

    uint32_t playbarCount = 1;

    // When the app is launched, we have a command line argument specifying to open a directory
    // We initialize a display to load and display its content
    if (parser.is_directory > 0)
    {
        Interface::Display* newDisplay = new Interface::Display(&profiler, &logger, &loader, 1);

        loader.Load(parser.path);
        loader.m_Medias[0].SetActive();
        loader.m_Medias[0].m_TimelineRange = loader.m_Medias[0].m_Range;
        loader.LoadImageToCache(0);
        
        playbarCount = loader.m_Medias[0].m_Range.y;

        newDisplay->Initialize(ocio);

        app.m_Displays[++app.m_DisplayCount] = newDisplay;
        app.m_ActiveDisplayID = 1;
    }
    else if (parser.is_file > 0)
    {
        Interface::Display* newDisplay = new Interface::Display(&profiler, &logger, &loader, 1);

        loader.Load(parser.path);
        loader.m_Medias[0].SetActive();
        loader.LoadImageToCache(0);
        
        newDisplay->Initialize(ocio);
        playbarCount = 0;

        app.m_Displays[++app.m_DisplayCount] = newDisplay;
        app.m_ActiveDisplayID = 1;
    }

    // initialize windows
    Interface::ImPlaybar playbar(&loader, ImVec2(0.0f, playbarCount));
    Interface::Menubar menubar;

    // Initialize memory profiler of main components
    profiler.MemUsage("Application Memory Usage", ToMB(GetCurrentRss()));
    // profiler.MemUsage("Display Memory Usage", ToMB((sizeof(display) + display.buffer_size) / 8));
    profiler.MemUsage("Ocio Module Memory Usage", ToMB((sizeof(ocio) + ocio.GetSize()) / 8));
    // profiler.MemUsage("Loader Memory Usage", ToMB((sizeof(loader) + loader.cached_size) / 8));
    
    bool change = false;

    // initialize ImFileDialog
    ifd::FileDialog::Instance().CreateTexture = [](uint8_t* data, int w, int h, char fmt) -> void* {
        GLuint tex;

        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, (fmt == 0) ? GL_BGRA : GL_RGBA, GL_UNSIGNED_BYTE, data));
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        return (void*)tex;
    };

    ifd::FileDialog::Instance().DeleteTexture = [](void* tex) {
        GLuint texID = (GLuint)tex;
        GL_CHECK(glDeleteTextures(1, &texID));
    };

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // update memory profiler
        profiler.MemUsage("Application Total Memory Usage", ToMB(GetCurrentRss()));
        profiler.MemUsage("Application Memory Usage", ToMB(GetCurrentRss()) - ToMB(loader.m_Cache->m_BytesSize));
        profiler.MemUsage("Ocio Module Memory Usage", ToMB((sizeof(ocio) + ocio.GetSize()) / 8));
        profiler.MemUsage("Cache Memory Usage", ToMB(loader.m_Cache->m_BytesSize));

        // The current media changed, reset the playbar and flush the cache
        if (mediaExplorerWindow.m_CurrentMediaChanged)
        {
            loader.m_Cache->Flush();
            playbar.SetRange(mediaExplorerWindow.m_CurrentMediaRange);
            mediaExplorerWindow.m_CurrentMediaChanged = false;
        }

        // Update the playbar
        if (app.m_DisplayCount > 0)
        {
            playbar.Update(&profiler);
        }
        
        uint32_t frameIndex = playbar.m_Frame;

        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

        // demo window for ImGui
        // if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);
        // ImPlot::ShowDemoWindow();

        // displays
        
        for(auto[id, display] : app.m_Displays)
        {
            if (change || playbar.m_Update) 
            {
                const auto startDpUpdate = profiler.Start();
                
                display->Update(ocio, frameIndex);
                change = false;
                playbar.m_Update = false;
                
                const auto endDpUpdate = profiler.End();
                profiler.Time("Displays Update", startDpUpdate, endDpUpdate);
            }

            display->Draw(frameIndex);

            // One info window per display
            const Core::Image currentImage = *app.m_Loader->GetImage(frameIndex);

            imageInfosWindow.Draw(currentImage, app.showImageInfosWindow);
            pixelInfosWindow.Draw(&loader, currentImage, display, app.showPixelInfosWindow);
        }

        // settings windows
        settings.Draw(playbar, &profiler, ocio, app);

        // menubar
        menubar.Draw(settings, app, playbar, ocio, profiler, change);

        // Media Explorer
        mediaExplorerWindow.Draw(&app, app.showMediaExplorerWindow);
    
        // playbar 
        playbar.Draw();

        // Rendering
        ImGui::Render();

        int display_w, display_h;

        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
    }

    // Release everything
    ocio.Release();
    loader.Release();
    app.Release();

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}