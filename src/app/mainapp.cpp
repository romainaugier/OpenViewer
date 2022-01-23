// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "mainapp.h"

int application(int argc, char** argv)
{
    // Setup exception handler
#ifdef OV_WIN
    SetUnhandledExceptionFilter(ExceptionHandler);    
#endif

    // Initialize the application
    printf("OpenViewer %s %s\n", OV_VERSION_STR, OV_PLATFORM_STR);

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
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

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
            application.UpdateCache();
            loader.LoadImageToCache(0, 0);
                                
            Interface::Display* newDisplay = new Interface::Display(application.m_Loader->m_Profiler, application.m_Logger, application.m_Loader, 1);

            newDisplay->Initialize(*application.m_OcioModule, 0);
            
            application.m_Displays[++application.m_DisplayCount] = std::make_pair(true, newDisplay);

        }
    }

    // Initialize the ui elements
    application.m_SettingsInterface.GetOcioConfig(ocio);

    Interface::ImageInfo imageInfosWindow;
    Interface::PixelInfo pixelInfosWindow;
    Interface::MediaExplorer mediaExplorerWindow(&loader, &logger);
    Interface::Menubar menubar;
    Interface::Scopes::Waveform waveform;
    waveform.Initialize();

    application.SetMenubar(&menubar);

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
        glfwPollEvents();

        // update memory profiler
        profiler.MemUsage("Application Total", ToMB(GetCurrentRss()));
        profiler.MemUsage("Application", ToMB(GetCurrentRss()) - ToMB(loader.m_Cache->m_BytesSize));
        profiler.MemUsage("Cache", ToMB(loader.m_Cache->m_BytesSize));

        // Update playbars
        for(auto[id, displayPair] : application.m_Displays)
        {
            Interface::Display* display = displayPair.second;

            display->UpdateAssociatedPlaybar();
        }

        // Update displays
        for(auto[id, displayPair] : application.m_Displays)
        {
            Interface::Display* display = displayPair.second;

            if (changeHappened || display->AssociatedPlaybar()->m_Update || application.SomethingChanged()) 
            {
                display->Update(ocio);

                display->AssociatedPlaybar()->NeedUpdate(false);   
                changeHappened = false;

                // waveform.Update(display->m_TransformedTexture, currentImage.m_Xres, currentImage.m_Yres);
            }
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

        // demo window for ImGui
        // if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);
        // ImPlot::ShowDemoWindow();

        // displays
        
        for(auto[id, displayPair] : application.m_Displays)
        {
            Interface::Display* display = displayPair.second;

            display->Draw();

            display->AssociatedPlaybar()->Draw();

            // One info window per display
            const Core::Image currentImage = *application.m_Loader->GetImage(display->m_MediaID, display->AssociatedPlaybar()->m_Frame);
            
            imageInfosWindow.Draw(currentImage, loader.GetMedia(display->m_MediaID), application.showImageInfosWindow);
            pixelInfosWindow.Draw(&loader, currentImage, display, application.showPixelInfosWindow);
        }

        // settings windows
        application.m_SettingsInterface.Draw(&profiler, &logger, ocio);

        // menubar
        menubar.Draw(application, ocio, profiler, changeHappened);

        // Media Explorer
        mediaExplorerWindow.Draw(&application, application.showMediaExplorerWindow);

        // Clear changes if any happened
        application.ClearChange();
        application.CacheSettingsChanged();
        application.UpdateDisplays();

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

        glfwSwapBuffers(window);
    }

    // Release everything
    application.Release();
    ocio.Release();
    loader.Release();

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}