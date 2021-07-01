// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#include "ocio.h"

void Ocio::Initialize()
{
    try
    {
        config = OCIO::Config::CreateFromFile("C:/Program Files/OCIO/dev/config-aces-reference.ocio");
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';

        // TODO implement config from file shipped with OpenViewer
    }

    GetOcioActiveDisplays();
    current_display = active_displays[0];

	GetOcioDisplayViews();
    current_view = active_views[0];

    GetRoles();
    current_role = roles[0];
}

void Ocio::GetOcioActiveViews() noexcept
{
    active_views.clear();

	const char* views = config->getActiveViews();
	const int views_count = config->getNumViews(current_display);
	const char s[2] = ",";
	
	active_views.reserve(views_count);

	char* token = strtok((char*)views, s);

	while (token != NULL)
	{
        std::string tmp = token;

        if (std::isspace(tmp[0])) tmp.erase(0, 1);
        if (std::isspace(tmp[tmp.size() - 1])) tmp.erase(tmp.size() - 1);

        char* temp = new char[tmp.size() + 1];
        memcpy(temp, tmp.c_str(), tmp.size() + 1);

		active_views.push_back(temp);
		token = strtok(NULL, s);
	}
}

// Little dirty method to get the list of views dependant of a display
void Ocio::GetOcioDisplayViews() noexcept
{
    active_views.clear();
    
    uint8_t i = 0;

    while (true)
    {
        std::string tmp = config->getView(current_display, i);
        
        if (tmp.empty()) break;

        if (std::isspace(tmp[0])) tmp.erase(0, 1);
        if (std::isspace(tmp[tmp.size() - 1])) tmp.erase(tmp.size() - 1);

        char* temp = new char[tmp.size() + 1];
        memcpy(temp, tmp.c_str(), tmp.size() + 1);

        active_views.push_back(temp);

        i++;
    }
}

void Ocio::GetOcioActiveDisplays() noexcept
{
    active_views.clear();

    const char* displays = config->getActiveDisplays();
    const int displays_count = config->getNumDisplays();
    const char s[2] = ",";

    active_displays.reserve(displays_count);

    char* token = strtok((char*)displays, s);

    while (token != NULL)
    {
        std::string tmp = token;

        if (std::isspace(tmp[0])) tmp.erase(0, 1);
        if (std::isspace(tmp[tmp.size() - 1])) tmp.erase(tmp.size() - 1);

        char* temp = new char[tmp.size() + 1];
        memcpy(temp, tmp.c_str(), tmp.size() + 1);

        active_displays.push_back(temp);
        token = strtok(NULL, s);
    }
}

void Ocio::GetRoles() noexcept
{
    uint8_t numroles = config->getNumRoles();

    for (uint8_t i = 0; i < numroles; i++)
    {
        std::string tmp = config->getRoleName(i);
        
        char* temp = new char[tmp.size() + 1];
        memcpy(temp, tmp.c_str(), tmp.size() + 1);

        roles.push_back(temp);
    }
}

void Ocio::ChangeConfig(const char* config_path)
{
    try
    {
        config = OCIO::Config::CreateFromFile(config_path);

        GetOcioActiveDisplays();
        current_display = active_displays[0];

        GetOcioActiveViews();
        current_view = active_views[0];
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << "\n";
    }
    
}

void Ocio::UpdateProcessor() 
{
    const char* display = config->getDisplay(current_display_idx);
    const char* view = config->getView(display, current_view_idx);

    try
    {
        OCIO::ConstProcessorRcPtr processor = config->getProcessor(current_role, current_display, current_view, OCIO::TRANSFORM_DIR_FORWARD);
        cpu = processor->getOptimizedCPUProcessor(OCIO::OPTIMIZATION_ALL);
    }
    catch (OCIO::Exception& exception)
    {
        std::cerr << "OCIO error : " << exception.what() << "\n";
    }
}

void Ocio::Process(float* const __restrict buffer, const uint16_t width, const uint16_t height)
{
    try
    {
        // apply the ocio view transform

        uint8_t numthreads = 8;
        
        // CPU
#pragma omp parallel for num_threads(numthreads)
        for (int8_t i = 0; i < numthreads; i++)
        {
            uint32_t index = i * ((width * height * 4) / numthreads);
            OCIO::PackedImageDesc img(&buffer[index], width, height / numthreads, 4);
            cpu->apply(img);
        }

        // GPU
        /*
        ogl_app = OCIO::OglApp::CreateOglApp("convert", 1, 1);
        ogl_app->printGLInfo();
        ogl_app->initImage(width, height, OCIO::OglApp::COMPONENTS_RGBA, buffer);
        ogl_app->createGLBuffers();
        
        const char* look = config->getDisplayViewLooks(current_display, current_view);

        OCIO::DisplayViewTransformRcPtr transform = OCIO::DisplayViewTransform::Create();
        transform->setSrc(OCIO::ROLE_SCENE_LINEAR);
        transform->setDisplay(current_display);
        transform->setView(current_view);

        OCIO::LegacyViewingPipelineRcPtr vpt = OCIO::LegacyViewingPipeline::Create();
        vpt->setDisplayViewTransform(transform);
        vpt->setLooksOverrideEnabled(true);
        vpt->setLooksOverride(look);

        OCIO::ConstProcessorRcPtr processor = vpt->getProcessor(config, config->getCurrentContext());
        OCIO::ConstGPUProcessorRcPtr gpu = processor->getDefaultGPUProcessor();

        OCIO::GpuShaderDescRcPtr shaderDesc = OCIO::GpuShaderDesc::CreateShaderDesc();
        shaderDesc->setLanguage(OCIO::GPU_LANGUAGE_GLSL_1_3);
        shaderDesc->setFunctionName("OCIODisplay");
        shaderDesc->setResourcePrefix("ocio_");

        gpu->extractGpuShaderInfo(shaderDesc);

        ogl_app->setShader(shaderDesc);
        ogl_app->redisplay();
        ogl_app->readImage(buffer);
        */
    }
    catch (OCIO::Exception& exception)
    {
        std::cerr << "OCIO Error : " << exception.what() << "\n";
    }
}

void Ocio::Release() noexcept
{
    for (auto& view : active_views)
    {
        delete[] view;
        view = nullptr;
    }

    for (auto& display : active_displays)
    {
        delete[] display;
        display = nullptr;
    }
}