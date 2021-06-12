#include "app.h"


int application(int argc, char** argv)
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;


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
    bool err = gl3wInit() != 0;

    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
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
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);

    // initialize ImFileDialog
    ifd::FileDialog::Instance().CreateTexture = [](uint8_t* data, int w, int h, char fmt) -> void* {
        GLuint tex;

        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, (fmt == 0) ? GL_BGRA : GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        return (void*)tex;
    };

    ifd::FileDialog::Instance().DeleteTexture = [](void* tex) {
        GLuint texID = (GLuint)tex;
        glDeleteTextures(1, &texID);
    };

    // initialize system
    Parser parser(argc, argv);

    Loader loader;

    bool initialize_display = false;

    if (parser.is_directory > 0)
    {
        loader.initialize(parser.path, 10000, true);
        initialize_display = true;
    }
    else if (parser.is_file > 0)
    {
        loader.initialize(parser.path, 0, false);
        initialize_display = true;
    }

    // initialize windows
    ImPlaybar playbar(ImVec2(0.0f, loader.count + 1.0f));

    Settings settings;
    Menubar menubar;
    Display display;

    if (initialize_display) display.init(loader);
    

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        
        loader.frame = playbar.playbar_frame;
        uint16_t frame_index = playbar.playbar_frame;

        if(loader.has_been_initialized > 0) loader.load_image(frame_index);
            
        display.update(loader, frame_index);

        /*
        if (!playbar.play)
        {
            loader.is_playing = 0;
            loader.load_image(frame_index);
            //if (loader.is_playloader_working == 1) loader.join_worker();
            //if (loader.has_finished == 1) loader.join_worker();

            glBindTexture(GL_TEXTURE_2D, tex);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, xres, yres, GL_RGB, GL_FLOAT, loader.single_image);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        else
        {
            loader.is_playing = 1;

            if (loader.cached[playbar.playbar_frame] < 1) playbar.playbar_frame++;
            if (loader.is_playloader_working == 0)
            {
                loader.launch_player_worker();
            }

            glBindTexture(GL_TEXTURE_2D, tex);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, xres, yres, GL_RGB, GL_FLOAT, &loader.memory_arena[loader.cache_stride * frame_index]);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        */

        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());


        // demo window for ImGui
        if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

        // display
        display.draw(loader, frame_index);

        // settings windows
        settings.draw(playbar);

        // playbar 
        ImGui::SetNextWindowBgAlpha(settings.interface_windows_bg_alpha);
        playbar.draw(loader.cached);

        // menubar
        menubar.draw(settings, loader, display);

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
    if(loader.has_finished > 0) loader.join_worker();
    loader.release();

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}