// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#include "app.h"


int application(int argc, char** argv)
{
    Logger logger;

    logger.setLevel(LogLevel_Debug);

    logger.Log(LogLevel_Debug, "Initializing OpenViewer...");

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
        logger.Log(LogLevel_Error, "OpenGL : Failed to initialize loader. Exiting application...");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
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

    profiler.current_memory_usage = ToMB(GetCurrentRss());

    Ocio ocio(&logger);
    ocio.Initialize();

    Loader loader(&profiler, &logger);
    Settings_Windows settings;

    bool initialize_display = false;

    if (parser.is_directory > 0)
    {
        uint64_t cache_size = static_cast<uint64_t>(settings.settings.cache_size) * 1000000;
        loader.Initialize(parser.path, cache_size, true);
        loader.LaunchSequenceWorker();
        initialize_display = true;
    }
    else if (parser.is_file > 0)
    {
        loader.Initialize(parser.path, 0, false);
        initialize_display = true;
    }

    // initialize windows
    ImPlaybar playbar(ImVec2(0.0f, loader.count + 1.0f));


    settings.GetOcioConfig(ocio);

    Menubar menubar;
    Display display(&profiler);

    if (initialize_display) display.Initialize(loader, ocio);

    // initialize memory profiler of main components
    profiler.current_memory_usage = ToMB(GetCurrentRss());
    profiler.display_size = ToMB((sizeof(display) + display.buffer_size) / 8);
    profiler.ocio_size = ToMB((sizeof(ocio) + ocio.GetSize()) / 8);
    profiler.loader_size = ToMB((sizeof(loader) + loader.cached_size) / 8);
    
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
        profiler.current_memory_usage = ToMB(GetCurrentRss());
        profiler.display_size = ToMB((sizeof(display) + display.buffer_size) / 8);
        profiler.ocio_size = ToMB((sizeof(ocio) + ocio.GetSize()) / 8);
        profiler.loader_size = ToMB((sizeof(loader) + loader.cached_size) / 8);

        loader.frame = playbar.playbar_frame;
        uint16_t frame_index = playbar.playbar_frame;

        
        if (loader.has_been_initialized > 0 && playbar.update > 0 ||
            loader.has_been_initialized > 0 && change)
        {
            if (settings.settings.use_cache) // cache loading
            {
                if (playbar.play < 1)
                {
                    loader.work_for_cache = false;
                    auto imgload_start = profiler.Start();
                    loader.is_playing = 0;

                    if (loader.cached[playbar.playbar_frame] < 1)
                    {
                        void* address = loader.UnloadImage();
                        loader.LoadImage(frame_index, address);
                    }

                    auto imgload_end = profiler.End();

                    profiler.Load(imgload_start, imgload_end);

                    display.Update(loader, ocio, frame_index);

                    change = false;
                }
                else
                {
                    auto imgload_start = profiler.Start();
                    loader.is_playing = 1;

                    if (loader.cached[(playbar.playbar_frame + 1) % loader.count] < 1) // emergency load
                    {
                        loader.mtx.lock();
                        loader.work_for_cache = 1;
                        loader.urgent_load = 1;
                        loader.cache_load_frame = playbar.playbar_frame;
                        loader.mtx.unlock();
                        loader.load_into_cache.notify_all();

                        // wait for a few ms to be make sure some frames are loaded
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                    else if (loader.cached[(playbar.playbar_frame + (loader.cache_size_count / 2)) % loader.count] < 1) // "casual" load 
                    {
                        loader.mtx.lock();
                        loader.work_for_cache = 1;
                        loader.cache_load_frame = playbar.playbar_frame;
                        loader.mtx.unlock();
                        loader.load_into_cache.notify_all();
                    }
                    if (loader.is_playloader_working == 0)
                    {
                        loader.LaunchPlayerWorker();
                    }
                    auto imgload_end = profiler.End();

                    profiler.Load(imgload_start, imgload_end);

                    display.Update(loader, ocio, frame_index);
                }
            }
            else // no cache allowed
            {
                if (loader.stop_playloader > 0) loader.stop_playloader = 1;

                auto imgload_start = profiler.Start();
                loader.is_playing = 0;

                void* address = loader.UnloadImage();
                loader.LoadImage(frame_index, address);

                auto imgload_end = profiler.End();

                profiler.Load(imgload_start, imgload_end);

                display.Update(loader, ocio, frame_index);

                change = false;
            }
        }
        

        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());


        // demo window for ImGui
        // if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

        // display
        display.Draw(loader, frame_index);

        // settings windows
        settings.draw(playbar, &profiler, ocio, loader);

        // playbar 
        ImGui::SetNextWindowBgAlpha(settings.settings.interface_windows_bg_alpha);
        playbar.draw(loader.cached);

        // menubar
        menubar.draw(settings, loader, display, playbar, ocio, profiler, change);

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

    // make sure to join remaining thread if it has not been     
    loader.stop_playloader = 1;

    if(loader.has_finished > 0 || loader.is_playloader_working > 0) loader.JoinWorker();

    loader.Release();
    display.Release();
    ocio.Release();
    settings.Release();

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}