// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "mainapp.h"

int application(int argc, char** argv)
{
    // Initialize the application
    printf("OpenViewer %s %s\n", OPENVIEWER_VERSION_STR, OPENVIEWER_PLATFORM_STR);

    // Logger
    Logger logger;
    logger.SetLevel(LogLevel_Diagnostic);
    logger.Log(LogLevel_Diagnostic, "[MAIN] : Initializing Logger");

    // Profiler
    Profiler profiler;
    
    // Ocio 
    Core::Ocio ocio(&logger);
    ocio.Initialize();

    // Loader/Cache
    Core::Loader loader(&logger, &profiler);
    loader.Initialize(false);
    
    // av_register_all();

    Interface::Application application(&logger, &loader, &ocio);

    // Setup GLFW
    glfwSetErrorCallback(GLFWErrorCallback);

    if (!glfwInit())
    {
        logger.Log(LogLevel_Error, "[GLFW] : Failed to initialize GLFW. Exiting application");
        std::exit(EXIT_FAILURE);
    }

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "OpenViewer", NULL, NULL);
    if (window == NULL)
    {
        logger.Log(LogLevel_Error, "[GLFW] : Failed to create window. Exiting application");
        std::exit(EXIT_FAILURE);
    }

    glfwSetWindowUserPointer(window, static_cast<void*>(&application));
    glfwSetDropCallback(window, GLFWDropEventCallback);
    glfwSetKeyCallback(window, GLFWKeyEventCallback);

    glfwMakeContextCurrent(window);
    // Enable vsync
    glfwSwapInterval(1);
    // Maximizes the window to the screen size
    glfwMaximizeWindow(window);

    // Initialize OpenGL loader
    const bool err = glewInit() != 0;

    if (err)
    {
        logger.Log(LogLevel_Error, "[OPENGL] : Failed to initialize loader. Exiting application");
        std::exit(EXIT_FAILURE);
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    Interface::DarkTheme();
    Interface::GetFonts();

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
    const ImVec4 clearColor = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    
    // Command line
    Core::CliParser parser(&logger);
    parser.ParseArgs(argc, argv);
    parser.ProcessArgs();

    if (parser.HasPaths())
    {
        std::vector<std::string> parsedPaths;
        parser.GetPaths(parsedPaths);

        for (const auto& path : parsedPaths)
        {
            loader.Load(path);
        }

        // If there is only one path in the cli, initialize a display to play it
        if (parsedPaths.size() == 1)
        {
            loader.SetMediaActive(0);
            loader.LoadImageToCache(0);
                                
            Interface::Display* newDisplay = new Interface::Display(application.m_Loader->m_Profiler, application.m_Logger, application.m_Loader, 1);

            newDisplay->Initialize(*application.m_OcioModule);
            
            application.m_Displays[++application.m_DisplayCount] = newDisplay;
            application.m_ActiveDisplayID = 1;
        }
    }

    // Initialize the ui elements
    Interface::Settings_Windows settings;
    settings.GetOcioConfig(ocio);

    Interface::ImageInfo imageInfosWindow;
    Interface::PixelInfo pixelInfosWindow;
    Interface::MediaExplorer mediaExplorerWindow(&loader, &logger);
    Interface::ImPlaybar playbar(&loader, ImVec2(0.0f, 1.0f));
    Interface::Menubar menubar;
    Interface::Plot::Parade rgbParade;
    rgbParade.Initialize();

    application.SetMenubar(&menubar);
    application.SetPlaybar(&playbar);

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
    
    bool changeHappened = false;

    // Main window loop
    while (!glfwWindowShouldClose(window))
    {
        // update memory profiler
        profiler.MemUsage("Application Total", ToMB(GetCurrentRss()));
        profiler.MemUsage("Application", ToMB(GetCurrentRss()) - ToMB(loader.m_Cache->m_BytesSize));
        profiler.MemUsage("Cache", ToMB(loader.m_Cache->m_BytesSize));

        // The current media changed, reset the playbar and flush the cache
        if (mediaExplorerWindow.m_CurrentMediaChanged)
        {
            loader.m_Cache->Flush();
            playbar.SetRange(mediaExplorerWindow.m_CurrentMediaRange);
            mediaExplorerWindow.m_CurrentMediaChanged = false;
        }

        // Update the playbar
        if (application.m_DisplayCount > 0)
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
        
        for(auto[id, display] : application.m_Displays)
        {
            if (changeHappened || playbar.m_Update) 
            {
                const auto startDpUpdate = profiler.Start();
                
                playbar.NeedUpdate(false);
                changeHappened = false;

                display->Update(ocio, frameIndex);
                
                const auto endDpUpdate = profiler.End();
                profiler.Time("Displays Update", startDpUpdate, endDpUpdate);

                rgbParade.Update(display->m_TransformedTexture);
            }

            display->Draw(frameIndex);

            // One info window per display
            const Core::Image currentImage = *application.m_Loader->GetImage(frameIndex);

            imageInfosWindow.Draw(currentImage, application.showImageInfosWindow);
            pixelInfosWindow.Draw(&loader, currentImage, display, application.showPixelInfosWindow);

            rgbParade.Draw();
        }

        // settings windows
        settings.Draw(playbar, &profiler, ocio, application);

        // menubar
        menubar.Draw(settings, application, playbar, ocio, profiler, changeHappened);

        // Media Explorer
        mediaExplorerWindow.Draw(&application, application.showMediaExplorerWindow);
    
        // playbar 
        playbar.Draw();

        // Rendering
        ImGui::Render();

        int display_w, display_h;

        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        application.HandleShortcuts();

        glfwSwapBuffers(window);
    }

    // Release everything
    playbar.Release();
    ocio.Release();
    loader.Release();
    application.Release();

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}