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
    
	GetOcioActiveViews();
}

void Ocio::GetOcioActiveViews() noexcept
{
	const char* views = config->getActiveViews();
	const char* display = config->getActiveDisplays();
    current_display = display;
	const int views_count = config->getNumViews(display);
	const char s[2] = ",";
	
	active_views.reserve(views_count);

	char* token = strtok((char*)views, s);

	while (token != NULL)
	{
		remove_spaces(token);
		active_views.push_back(token);
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

void Ocio::Process(float* __restrict buffer, const uint16_t width, const uint16_t height)
{
    try
    {
        // apply the ocio view transform

        // CPU
        OCIO::ConstProcessorRcPtr processor = config->getProcessor(OCIO::ROLE_SCENE_LINEAR, current_display, current_view, OCIO::TRANSFORM_DIR_FORWARD);
        OCIO::ConstCPUProcessorRcPtr cpu = processor->getOptimizedCPUProcessor(OCIO::OPTIMIZATION_ALL);

        OCIO::PackedImageDesc img(buffer, width, height, 4);
        cpu->apply(img);
        
        // GPU
        /*
        const char* display = config->getDefaultDisplay();
        const char* view = config->getDefaultView(display);
        const char* look = config->getDisplayViewLooks(display, view);

        OCIO::DisplayViewTransformRcPtr transform = OCIO::DisplayViewTransform::Create();
        transform->setSrc(OCIO::ROLE_SCENE_LINEAR);
        transform->setDisplay(display);
        transform->setView(view);

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

        OCIO::OglAppRcPtr oglApp = std::make_shared<OCIO::ScreenApp>("ociodisplay", 512, 512);
        oglApp->initImage(width, height, OCIO::OglApp::COMPONENTS_RGBA, buffer);
        oglApp->setShader(shaderDesc);

        //oglApp->redisplay();
        */
    }
    catch (OCIO::Exception& exception)
    {
        std::cerr << "OCIO Error : " << exception.what() << "\n";
    }
}