// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "ocio.h"

namespace Core
{
    uint32_t Ocio::GetSize() const noexcept
    {
        uint32_t size = 0;

        for (auto& view : views) size += sizeof(view) * strlen(view);
        for (auto& display : displays) size += sizeof(display) * strlen(display);
        for (auto& role : roles) size += sizeof(role) * strlen(role);
        for (auto& look : looks) size += sizeof(look) * strlen(look);

        size += sizeof(config_path) * strlen(config_path);
        size += sizeof(current_view) * strlen(current_view);
        size += sizeof(current_display) * strlen(current_display);
        size += sizeof(current_role) * strlen(current_role);
        size += sizeof(current_look) * strlen(current_look);

        return size;
    }

    void Ocio::Initialize()
    {
        this->logger->Log(LogLevel_Diagnostic, "[OCIO] : Initializing OCIO");

        config_path = std::getenv("OCIO");

        std::string current_path = std::filesystem::current_path().string();

        char default_config_path[4096];
        sprintf(default_config_path, "%s/configs/default.ocio", current_path.c_str());
        
        if(config_path != nullptr)
        {
            try
            {
                config = OCIO::Config::CreateFromEnv();

                logger->Log(LogLevel_Message, "[OCIO] : Configuration loaded from file %s", config_path);
            }
            catch(const std::exception& e)
            {
                logger->Log(LogLevel_Warning, "[OCIO] : %s Using default shipped configuration", e.what());

                config = OCIO::Config::CreateFromFile(default_config_path);
                config_path = default_config_path;
            }
        }
        else
        {
            try
            {
                logger->Log(LogLevel_Warning, "[OCIO] : Environment variable can't be found. Using default shipped configuration");
                config = OCIO::Config::CreateFromFile(default_config_path);
                config_path = default_config_path;
            }
            catch(const std::exception& e)
            {
                logger->Log(LogLevel_Error, "[OCIO] : %s", e.what());
                std::exit(EXIT_FAILURE);
            }
        }


        GetOcioActiveDisplays();
        current_display = displays[0];

        GetOcioDisplayViews();
        current_view = views[0];

        GetRoles();
        current_role = roles[0];

        GetLooks();
        current_look = looks[0];
    }

    void Ocio::GetOcioActiveViews() noexcept
    {
        for (auto& view : views)
        {
            delete[] view;
            view = nullptr;
        }

        views.clear();

        const char* active_views = config->getActiveViews();
        const int views_count = config->getNumViews(current_display);
        const char s[2] = ",";
        
        views.reserve(views_count);

        char* token = strtok((char*)active_views, s);

        while (token != NULL)
        {
            std::string tmp = token;

            if (std::isspace(tmp[0])) tmp.erase(0, 1);
            if (std::isspace(tmp[tmp.size() - 1])) tmp.erase(tmp.size() - 1);

            char* temp = new char[tmp.size() + 1];
            memcpy(temp, tmp.c_str(), tmp.size() + 1);

            views.push_back(temp);
            token = strtok(NULL, s);
        }
    }

    // Little dirty method to get the list of views dependant of a display
    void Ocio::GetOcioDisplayViews() noexcept
    {
        for (auto& view : views)
        {
            delete[] view;
            view = nullptr;
        }

        views.clear();
        
        uint8_t i = 0;

        while (true)
        {
            std::string tmp = config->getView(current_display, i);
            
            if (tmp.empty()) break;

            if (std::isspace(tmp[0])) tmp.erase(0, 1);
            if (std::isspace(tmp[tmp.size() - 1])) tmp.erase(tmp.size() - 1);

            char* temp = new char[tmp.size() + 1];
            memcpy(temp, tmp.c_str(), tmp.size() + 1);

            views.push_back(temp);

            i++;
        }
    }

    void Ocio::GetOcioActiveDisplays() noexcept
    {
        for (auto& display : displays)
        {
            delete[] display;
            display = nullptr;
        }

        displays.clear();

        const char* active_displays = config->getActiveDisplays();
        const int displays_count = config->getNumDisplays();
        const char s[2] = ",";

        displays.reserve(displays_count);

        char* token = strtok((char*)active_displays, s);

        while (token != NULL)
        {
            std::string tmp = token;

            if (std::isspace(tmp[0])) tmp.erase(0, 1);
            if (std::isspace(tmp[tmp.size() - 1])) tmp.erase(tmp.size() - 1);

            char* temp = new char[tmp.size() + 1];
            memcpy(temp, tmp.c_str(), tmp.size() + 1);

            displays.push_back(temp);
            token = strtok(NULL, s);
        }
    }

    void Ocio::GetRoles() noexcept
    {
        for (auto& role : roles)
        {
            delete[] role;
            role = nullptr;
        }

        roles.clear();

        const uint8_t numroles = config->getNumRoles();

        roles.reserve(numroles);

        for (uint8_t i = 0; i < numroles; i++)
        {
            std::string tmp = config->getRoleColorSpace(i);
            const std::string tmp2 = config->getRoleName(i);

            tmp = tmp2 + " : " + tmp;
            
            char* temp = new char[tmp.size() + 1];
            memcpy(temp, tmp.c_str(), tmp.size() + 1);

            roles.push_back(temp);
        }
    }

    void Ocio::GetLooks() noexcept
    {
        for (auto& look : looks)
        {
            if (strcmp(look, "None") == 0) continue;
            else
            {
                delete[] look;
                look = nullptr;
            }
        }

        looks.clear();

        const uint8_t numlooks = config->getNumLooks();

        looks.reserve(numlooks);

        looks.emplace_back("None");

        for (uint8_t i = 0; i < numlooks; i++)
        {
            std::string tmp = config->getLookNameByIndex(i);

            if (std::isspace(tmp[0])) tmp.erase(0, 1);
            if (std::isspace(tmp[tmp.size() - 1])) tmp.erase(tmp.size() - 1);

            char* temp = new char[tmp.size() + 1];
            memcpy(temp, tmp.c_str(), tmp.size() + 1);

            looks.push_back(temp);
        }
    }

    void Ocio::ChangeConfig(const char* config_path)
    {
        try
        {
            config = OCIO::Config::CreateFromFile(config_path);
            config_path = config_path;

            logger->Log(LogLevel_Message, "[OCIO] : Configuration switched to %s", config_path);

            GetOcioActiveDisplays();
            current_display = displays[0];

            GetOcioActiveViews();
            current_view = views[0];

            GetRoles();
            current_role = roles[0];

            GetLooks();
            current_look = looks[0];
        }
        catch (const std::exception& e)
        {
            logger->Log(LogLevel_Warning, "[OCIO] : %s. Keeping the current configuration", e.what());
        }
        
    }

    void Ocio::UpdateProcessor()
    {
        const char* display = config->getDisplay(current_display_idx);
        const char* view = config->getView(display, current_view_idx);
        const char* role = config->getRoleColorSpace(current_role_idx);

        logger->Log(LogLevel_Debug, "[OCIO] : Updating processor to use %s | %s | %s", role, display, view);

        // Create the view pipeline
        try
        {
            const OCIO::DisplayViewTransformRcPtr transform = OCIO::DisplayViewTransform::Create();
            transform->setSrc(role);
            transform->setDisplay(display);
            transform->setView(view);
            
            const OCIO::LegacyViewingPipelineRcPtr vp = OCIO::LegacyViewingPipeline::Create();
            vp->setDisplayViewTransform(transform);

            if (current_look_idx != 0)
            {
                vp->setLooksOverrideEnabled(true);
                vp->setLooksOverride(current_look);
            }
            else
            {
                vp->setLooksOverrideEnabled(false);
            }

            // Exposure modification
            {
                const double gain = powf(2.0f, static_cast<double>(exposure_stops));
                const double slope4f[] = { gain, gain, gain, 1.0 };
                double m44[16];
                double offset4[4];
                OCIO::MatrixTransform::Scale(m44, offset4, slope4f);
                OCIO::MatrixTransformRcPtr mtx = OCIO::MatrixTransform::Create();
                mtx->setMatrix(m44);
                mtx->setOffset(offset4);
                vp->setLinearCC(mtx);
            }

            // Channel swizzling
            {
                double lumacoef[3];
                config->getDefaultLumaCoefs(lumacoef);
                double m44[16];
                double offset[4];
                OCIO::MatrixTransform::View(m44, offset, channel_hot, lumacoef);
                OCIO::MatrixTransformRcPtr swizzle = OCIO::MatrixTransform::Create();
                swizzle->setMatrix(m44);
                swizzle->setOffset(offset);
                vp->setChannelView(swizzle);
            }

            // Post display gamma
            {
                double exponent = 1.0f / std::max(0.01, static_cast<double>(gamma));
                const double exponent4f[] = { exponent, exponent, exponent, 1.0 };
                OCIO::ExponentTransformRcPtr exponent_transform = OCIO::ExponentTransform::Create();
                exponent_transform->setValue(exponent4f);
                vp->setDisplayCC(exponent_transform);
            }

            OCIO::ConstProcessorRcPtr processor = vp->getProcessor(config, config->getCurrentContext());
            
            // if we use the gpu, create and compile the shader needed for the transform
            if (use_gpu > 0)
            {
                OCIO::GpuShaderDescRcPtr shaderDesc;
                shaderDesc = OCIO::GpuShaderDesc::CreateShaderDesc();
                shaderDesc->setLanguage(OCIO::GPU_LANGUAGE_GLSL_1_3);
                shaderDesc->setFunctionName("OCIODisplay");
                shaderDesc->setResourcePrefix("ocio_");

                gpu = processor->getOptimizedGPUProcessor(OCIO::OPTIMIZATION_ALL);

                gpu->extractGpuShaderInfo(shaderDesc);

                ogl_builder = OCIO::OpenGLBuilder::Create(shaderDesc);

                ogl_builder->allocateAllTextures(1);

                std::ostringstream frag;
                frag << std::endl
                    << "uniform sampler2D img;" << std::endl
                    << std::endl
                    << "void main()" << std::endl
                    << "{" << std::endl
                    << "    vec4 col = texture2D(img, gl_TexCoord[0].st);" << std::endl
                    << "    gl_FragColor = " << shaderDesc->getFunctionName() << "(col);" << std::endl
                    << "}" << std::endl;

                ogl_builder->buildProgram(frag.str().c_str());
                ogl_builder->useProgram();
                glUniform1i(glGetUniformLocation(ogl_builder->getProgramHandle(), "img"), 0);
                ogl_builder->useAllTextures();
                ogl_builder->useAllUniforms();
            }
            else
            // use the cpu processor
            {
                cpu = processor->getOptimizedCPUProcessor(OCIO::OPTIMIZATION_ALL);
            }
        }
        catch (OCIO::Exception& exception)
        {
            logger->Log(LogLevel_Warning, "[OCIO] : %s", exception.what());

            const char* display = config->getDefaultDisplay();
            const char* view = config->getDefaultView(display);

            OCIO::ConstProcessorRcPtr processor = config->getProcessor(OCIO::ROLE_SCENE_LINEAR, display, view, OCIO::TRANSFORM_DIR_FORWARD);
            
            // if we use the gpu, create and compile the shader needed for the transform
            if (use_gpu > 0)
            {
                OCIO::GpuShaderDescRcPtr shaderDesc;
                GL_CHECK(shaderDesc = OCIO::GpuShaderDesc::CreateShaderDesc());
                shaderDesc->setLanguage(OCIO::GPU_LANGUAGE_GLSL_1_3);
                shaderDesc->setFunctionName("OCIODisplay");
                shaderDesc->setResourcePrefix("ocio_");

                gpu = processor->getOptimizedGPUProcessor(OCIO::OPTIMIZATION_ALL);

                gpu->extractGpuShaderInfo(shaderDesc);

                ogl_builder = OCIO::OpenGLBuilder::Create(shaderDesc);

                ogl_builder->allocateAllTextures(1);

                std::ostringstream frag;
                frag << std::endl
                    << "uniform sampler2D img;" << std::endl
                    << std::endl
                    << "void main()" << std::endl
                    << "{" << std::endl
                    << "    vec4 col = texture2D(img, gl_TexCoord[0].st);" << std::endl
                    << "    gl_FragColor = " << shaderDesc->getFunctionName() << "(col);" << std::endl
                    << "}" << std::endl;

                ogl_builder->buildProgram(frag.str().c_str());
                ogl_builder->useProgram();
                glUniform1i(glGetUniformLocation(ogl_builder->getProgramHandle(), "img"), 0);
                ogl_builder->useAllTextures();
                ogl_builder->useAllUniforms();
            }
            else
            // use the cpu processor, much slower
            {
                cpu = processor->getOptimizedCPUProcessor(OCIO::OPTIMIZATION_ALL);
            }
        }
    }

    void Ocio::Process(const uint16_t width, const uint16_t height)
    {
        ogl_builder->useAllUniforms();
        // try
        // {
        //     // apply the ocio view transform

        //     // GPU
        //     if (use_gpu > 0) ogl_builder->useAllUniforms();
        //     else logger->Log(LogLevel_Error, "[OCIO] : CPU Processor has not been implemented yet");
        // }
        // catch (OCIO::Exception& exception)
        // {
        //     logger->Log(LogLevel_Error, "[OCIO] : %s", exception.what());
        // }
    }

    // cleanup the vectors containing views, displays, roles and looks
    void Ocio::Release() noexcept
    {
        for (auto& view : views)
        {
            delete[] view;
            view = nullptr;
        }

        for (auto& display : displays)
        {
            delete[] display;
            display = nullptr;
        }

        for (auto& role : roles)
        {
            delete[] role;
            role = nullptr;
        }

        for (auto& look : looks)
        {
            if (strcmp(look, "None") == 0) continue;
            else
            {
                delete[] look;
                look = nullptr;
            }
        }

        logger->Log(LogLevel_Diagnostic, "[OCIO] : Released OCIO");
    }
} // End namespace Core