// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#include "ocio.h"

void Ocio::Initialize()
{
    try
    {
        config = OCIO::GetCurrentConfig();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';

        // TODO implement config from file shipped with OpenViewer
    }
    
    GetOcioActiveDisplays();
    current_display = active_displays[0].c_str();

	GetOcioActiveViews();
    current_view = active_views[0].c_str();
}

void Ocio::GetOcioActiveViews() noexcept
{
	const char* views = config->getActiveViews();
	const int views_count = config->getNumViews(current_display);
	const char s[2] = ",";
	
	active_views.reserve(views_count);

	char* token = strtok((char*)views, s);

	while (token != NULL)
	{
        //std::string str_token = token;
        /*str_token.erase(0, 1);
        str_token.erase(str_token.end() - 1);

        char* new_token = (char*)str_token.c_str();

        printf("%s\n", new_token);*/

		active_views.push_back(token);
		token = strtok(NULL, s);
	}
}

void Ocio::GetOcioActiveDisplays() noexcept
{
    const char* displays = config->getActiveDisplays();
    const int displays_count = config->getNumDisplays();
    const char s[2] = ",";

    active_displays.reserve(displays_count);

    char* token = strtok((char*)displays, s);

    while (token != NULL)
    {
        remove_spaces(token);
        active_displays.push_back(token);
        token = strtok(NULL, s);
    }
}

void Ocio::ChangeConfig(const char* config_path)
{
    try
    {
        config = OCIO::Config::CreateFromFile(config_path);
        GetOcioActiveViews();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << "\n";
    }
    
}

void Ocio::UpdateProcessor() noexcept
{
    OCIO::ConstProcessorRcPtr processor = config->getProcessor(OCIO::ROLE_SCENE_LINEAR, current_display, current_view, OCIO::TRANSFORM_DIR_FORWARD);
    cpu = processor->getOptimizedCPUProcessor(OCIO::OPTIMIZATION_ALL);
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