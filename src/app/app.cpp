// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "app.h"

namespace Interface
{
    Application::Application(Logger* logger)
    {
        this->m_Logger = logger;
    }
}

int application(int argc, char** argv)
{
    // Initialize the application
    Logger logger;
    logger.SetLevel(LogLevel_Debug);
    logger.Log(LogLevel_Debug, "Initializing OpenViewer...");

    Interface::Application app(&logger);

    // Setup window
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit())
    {
        logger.Log(LogLevel_Error, "GLFW : Failed to initialize GLFW. Exiting application...");
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
        logger.Log(LogLevel_Error, "OPENGL Failed to initialize loader. Exiting application...");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // setup Dear ImGui style
    ImGui::StyleColorsDark();

    // docking
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // style
    ImGuiStyle* style = &ImGui::GetStyle();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    GL_CHECK(ImGui_ImplOpenGL3_Init(glsl_version));

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);

    // initialize system
    Parser parser(argc, argv);
    Profiler profiler;

    // profiler.current_memory_usage = ToMB(GetCurrentRss());

    Core::Ocio ocio(&logger);
    ocio.Initialize();

    Interface::Settings_Windows settings;

    uint16_t playbarCount = 1;

    // When the app is launched, we have a command line argument specifying to open a directory
    // We initialize a display to load and display its content
    if (parser.is_directory > 0)
    {
        Interface::Display* newDisplay = new Interface::Display(&profiler, &logger);

        newDisplay->Initialize(ocio);
        newDisplay->m_Loader->Initialize(parser.path, false, 0);
        playbarCount = newDisplay->m_Loader->m_ImageCount;

        app.m_Displays[0] = newDisplay;
    }
    else if (parser.is_file > 0)
    {
        Interface::Display* newDisplay = new Interface::Display(&profiler, &logger);

        newDisplay->Initialize(ocio);
        newDisplay->m_Loader->Initialize(parser.path);
        playbarCount = newDisplay->m_Loader->m_ImageCount;

        app.m_Displays[0] = newDisplay;
    }

    // initialize windows
    ImPlaybar playbar(ImVec2(0.0f, playbarCount + 1.0f));

    settings.GetOcioConfig(ocio);
    
    Interface::Menubar menubar;

    // initialize memory profiler of main components
    profiler.MemUsage("Application Memory Usage", ToMB(GetCurrentRss()));
    // profiler.MemUsage("Display Memory Usage", ToMB((sizeof(display) + display.buffer_size) / 8));
    profiler.MemUsage("Ocio Module Memory Usage", ToMB((sizeof(ocio) + ocio.GetSize()) / 8));
    // profiler.MemUsage("Loader Memory Usage", ToMB((sizeof(loader) + loader.cached_size) / 8));
    
    bool change = true;

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
        // if (loader.has_finished > 0 ) loader.JoinWorker();

        // update memory profiler
        profiler.MemUsage("Application Memory Usage", ToMB(GetCurrentRss()));
        // profiler.MemUsage("Display Memory Usage", ToMB((sizeof(display) + display.buffer_size) / 8));
        profiler.MemUsage("Ocio Module Memory Usage", ToMB((sizeof(ocio) + ocio.GetSize()) / 8));
        // profiler.MemUsage("Loader Memory Usage", ToMB((sizeof(loader) + loader.cached_size) / 8));

        uint16_t frame_index = playbar.playbar_frame;

        
        // if (loader.has_been_initialized > 0 && playbar.update > 0 ||
        //     loader.has_been_initialized > 0 && change)
        // {
        //     if (settings.settings.use_cache) // cache loading
        //     {
        //         if (playbar.play < 1)
        //         {
        //             loader.work_for_cache = false;
        //             auto imgload_start = profiler.Start();
        //             loader.is_playing = 0;

        //             if (loader.cached[playbar.playbar_frame] < 1)
        //             {
        //                 void* address = loader.UnloadImage();

        //                 if (address == nullptr) address = loader.memory_arena;
                        
        //                 loader.LoadImage(frame_index, address);
        //             }

        //             auto imgload_end = profiler.End();

        //             profiler.Time("Image Loading Time", imgload_start, imgload_end);

        //             display.Update(loader, ocio, frame_index);

        //             if (settings.settings.parade)
        //             {
        //                 auto plotstart = profiler.Start();
        //                 display.GetDisplayPixels();
        //                 // plot.Update(display.buffer);
        //                 auto plotend = profiler.End();
        //                 profiler.Time("Plot Time", plotstart, plotend);
        //             }

        //             change = false;
        //         }
        //         else
        //         {
        //             auto imgload_start = profiler.Start();
        //             loader.is_playing = 1;

        //             if (loader.cached[(playbar.playbar_frame + 1) % loader.count] < 1) // emergency load
        //             {
        //                 loader.mtx.lock();
        //                 loader.work_for_cache = 1;
        //                 loader.urgent_load = 1;
        //                 loader.cache_load_frame = playbar.playbar_frame;
        //                 loader.mtx.unlock();
        //                 loader.load_into_cache.notify_all();

        //                 // wait for a few ms to be make sure some frames are loaded
        //                 std::this_thread::sleep_for(std::chrono::milliseconds(50));
        //             }
        //             else if (loader.cached[(playbar.playbar_frame + (loader.cache_size_count / 2)) % loader.count] < 1) // "casual" load 
        //             {
        //                 loader.mtx.lock();
        //                 loader.work_for_cache = 1;
        //                 loader.cache_load_frame = playbar.playbar_frame;
        //                 loader.mtx.unlock();
        //                 loader.load_into_cache.notify_all();
        //             }
        //             if (loader.is_playloader_working < 1)
        //             {
        //                 loader.stop_playloader = 0;
        //                 loader.LaunchPlayerWorker();
        //             }
        //             auto imgload_end = profiler.End();

        //             profiler.Time("Image Loading Time", imgload_start, imgload_end);

        //             display.Update(loader, ocio, frame_index);
                    
        //             if (settings.settings.parade)
        //             {
        //                 auto plotstart = profiler.Start();
        //                 display.GetDisplayPixels();
        //                 // plot.Update(display.buffer);
        //                 auto plotend = profiler.End();
        //                 profiler.Time("Plot Time", plotstart, plotend);
        //             }

        //             change = false;
        //         }
        //     }
        //     else // no cache allowed
        //     {
        //         if (loader.stop_playloader > 0) loader.stop_playloader = 1;

        //         auto imgload_start = profiler.Start();
        //         loader.is_playing = 0;

        //         if (loader.cached[frame_index] == 0)
        //         {
        //             void* address = loader.UnloadImage();
        //             loader.LoadImage(frame_index, address);
        //         }

        //         auto imgload_end = profiler.End();

        //         profiler.Time("Image Loading Time", imgload_start, imgload_end);

        //         display.Update(loader, ocio, frame_index);

        //         if (settings.settings.parade)
        //         {
        //             display.GetDisplayPixels();
        //             auto plotstart = profiler.Start();
        //             // plot.Update(display.buffer);
        //             auto plotend = profiler.End();
        //             profiler.Time("Plot Time", plotstart, plotend);
        //         }

        //         change = false;
        //     }
        // }

        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

        // demo window for ImGui
        // if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);
        // ImPlot::ShowDemoWindow();

        // display
        // display.Draw(loader, frame_index);

        // settings windows
        // settings.Draw(playbar, &profiler, ocio, loader);

        // menubar
        // menubar.Draw(settings, loader, display, playbar, ocio, profiler, change);
        
        // playbar 
        ImGui::SetNextWindowBgAlpha(settings.settings.interface_windows_bg_alpha);
        //playbar.draw(loader.cached);

        // plot
        if (settings.settings.parade)
        {
            auto plot_draw_start = profiler.Start();
            auto plot_draw_end = profiler.End();
            profiler.Time("Plot Drawing Time", plot_draw_start, plot_draw_end);
        }

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

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}