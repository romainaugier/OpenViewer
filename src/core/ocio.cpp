// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "ocio.h"

namespace Core
{
    void Ocio::Initialize()
    {
        this->m_Logger->Log(LogLevel_Diagnostic, "[OCIO] : Initializing OCIO");

        const char* envPath = std::getenv("OCIO");

        const std::string currentPath = std::filesystem::current_path().string();

        char defaultConfigPath[4096];
        Utils::Str::Format(defaultConfigPath, "%s/configs/default.ocio", currentPath.c_str());
        
        if(envPath != nullptr)
        {
            try
            {
                this->m_Config = OCIO::Config::CreateFromEnv();

                this->m_Logger->Log(LogLevel_Message, "[OCIO] : Configuration loaded from file %s", this->m_ConfigPath);
            }
            catch(const std::exception& e)
            {
                this->m_Logger->Log(LogLevel_Warning, "[OCIO] : %s Using default shipped configuration", e.what());

                this->m_Config = OCIO::Config::CreateFromFile(defaultConfigPath);
                this->m_ConfigPath = defaultConfigPath;
            }
        }
        else
        {
            try
            {
                this->m_Logger->Log(LogLevel_Warning, "[OCIO] : Environment variable can't be found. Using default shipped configuration");
                this->m_Config = OCIO::Config::CreateFromFile(defaultConfigPath);
                this->m_ConfigPath = defaultConfigPath;
            }
            catch(const std::exception& e)
            {
                this->m_Logger->Log(LogLevel_Error, "[OCIO] : %s", e.what());
                std::exit(EXIT_FAILURE);
            }
        }


        GetOcioActiveDisplays();
        this->m_CurrentDisplay = this->m_Displays[0];

        GetOcioDisplayViews();
        this->m_CurrentView = this->m_Views[0];

        GetRoles();
        this->m_CurrentRole = this->m_Roles[0];

        GetLooks();
        this->m_CurrentLook = this->m_Looks[0];
    }

    void Ocio::GetOcioActiveViews() noexcept
    {
        this->m_Views.clear();

        const char* activeViews = this->m_Config->getActiveViews();
        const int viewsCount = this->m_Config->getNumViews(m_CurrentDisplay.c_str());
        
        std::vector<std::string> views;
        views.reserve(viewsCount);

        Utils::Str::Split(views, activeViews, ',');

        for (auto& view : views) Utils::Str::Replace(view, " ", "");

        this->m_Views = std::move(views);
    }

    // Little dirty method to get the list of views dependant of a display
    void Ocio::GetOcioDisplayViews() noexcept
    {
        this->m_Views.clear();
        
        uint16_t i = 0;

        while (true)
        {
            std::string tmp = this->m_Config->getView(this->m_CurrentDisplay.c_str(), i);
            
            if (tmp.empty()) break;

            Utils::Str::Replace(tmp, " ", "");

            this->m_Views.push_back(tmp);

            ++i;
        }
    }

    void Ocio::GetOcioActiveDisplays() noexcept
    {
        this->m_Displays.clear();

        const char* activeDisplays = this->m_Config->getActiveDisplays();
        const int displaysCount = this->m_Config->getNumDisplays();

        std::vector<std::string> displays;
        displays.reserve(displaysCount);

        Utils::Str::Split(displays, activeDisplays, ',');

        for (auto& display : displays) Utils::Str::Replace(display, " ", "");

        this->m_Displays = std::move(displays);
    }

    void Ocio::GetRoles() noexcept
    {
        this->m_Roles.clear();

        const uint16_t numRoles = this->m_Config->getNumRoles();

        this->m_Roles.reserve(numRoles);

        for (uint16_t i = 0; i < numRoles; i++)
        {
            const std::string tmp = m_Config->getRoleColorSpace(i);
            const std::string tmp2 = m_Config->getRoleName(i);

            m_Roles.emplace_back(tmp + " : " + tmp2);
        }
    }

    void Ocio::GetLooks() noexcept
    {
        m_Looks.clear();

        const uint16_t numLooks = this->m_Config->getNumLooks();

        m_Looks.reserve(numLooks);

        m_Looks.emplace_back("None");

        for (uint16_t i = 0; i < numLooks; i++)
        {
            std::string tmp = this->m_Config->getLookNameByIndex(i);

            Utils::Str::Replace(tmp, " ", "");

            m_Looks.push_back(tmp);
        }
    }

    void Ocio::ChangeConfig(const char* config_path)
    {
        try
        {
            this->m_Config = OCIO::Config::CreateFromFile(config_path);

            this->m_Logger->Log(LogLevel_Message, "[OCIO] : Configuration switched to %s", config_path);

            this->GetOcioActiveDisplays();
            this->m_CurrentDisplay = this->m_Displays[0];

            this->GetOcioActiveViews();
            this->m_CurrentView = this->m_Views[0];

            this->GetRoles();
            this->m_CurrentRole = this->m_Roles[0];

            this->GetLooks();
            this->m_CurrentLook = this->m_Looks[0];
        }
        catch (const std::exception& e)
        {
            this->m_Logger->Log(LogLevel_Warning, "[OCIO] : %s. Keeping the current configuration", e.what());
        }
    }

    void Ocio::UpdateProcessor()
    {
        const char* display = this->m_Config->getDisplay(this->m_CurrentDisplayIdx);
        const char* view = this->m_Config->getView(display, this->m_CurrentViewIdx);
        const char* role = this->m_Config->getRoleColorSpace(this->m_CurrentRoleIdx);

        this->m_Logger->Log(LogLevel_Debug, "[OCIO] : Updating processor to use %s | %s | %s", role, display, view);

        // Create the view pipeline
        try
        {
            const OCIO::DisplayViewTransformRcPtr transform = OCIO::DisplayViewTransform::Create();
            transform->setSrc(role);
            transform->setDisplay(display);
            transform->setView(view);
            
            const OCIO::LegacyViewingPipelineRcPtr vp = OCIO::LegacyViewingPipeline::Create();
            vp->setDisplayViewTransform(transform);

            if (this->m_CurrentLookIdx != 0)
            {
                vp->setLooksOverrideEnabled(true);
                vp->setLooksOverride(this->m_CurrentLook.c_str());
            }
            else
            {
                vp->setLooksOverrideEnabled(false);
            }

            // Exposure modification
            {
                const double gain = powf(2.0f, static_cast<double>(this->m_ExposureStops));
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
                m_Config->getDefaultLumaCoefs(lumacoef);
                double m44[16];
                double offset[4];
                OCIO::MatrixTransform::View(m44, offset, this->m_ChannelsHot, lumacoef);
                OCIO::MatrixTransformRcPtr swizzle = OCIO::MatrixTransform::Create();
                swizzle->setMatrix(m44);
                swizzle->setOffset(offset);
                vp->setChannelView(swizzle);
            }

            // Post display gamma
            {
                double exponent = 1.0f / std::max(0.01, static_cast<double>(this->m_Gamma));
                const double exponent4f[] = { exponent, exponent, exponent, 1.0 };
                OCIO::ExponentTransformRcPtr exponent_transform = OCIO::ExponentTransform::Create();
                exponent_transform->setValue(exponent4f);
                vp->setDisplayCC(exponent_transform);
            }

            OCIO::ConstProcessorRcPtr processor = vp->getProcessor(this->m_Config, this->m_Config->getCurrentContext());
            
            // if we use the gpu, create and compile the shader needed for the transform
            if (this->m_UseGPU)
            {
                OCIO::GpuShaderDescRcPtr shaderDesc;
                shaderDesc = OCIO::GpuShaderDesc::CreateShaderDesc();
                shaderDesc->setLanguage(OCIO::GPU_LANGUAGE_GLSL_1_3);
                shaderDesc->setFunctionName("OCIODisplay");
                shaderDesc->setResourcePrefix("ocio_");

                this->m_GPU = processor->getOptimizedGPUProcessor(OCIO::OPTIMIZATION_ALL);

                this->m_GPU->extractGpuShaderInfo(shaderDesc);

                this->m_OGLBuilder = OCIO::OpenGLBuilder::Create(shaderDesc);

                this->m_OGLBuilder->allocateAllTextures(1);

                std::ostringstream frag;
                frag << std::endl
                    << "uniform sampler2D img;" << std::endl
                    << std::endl
                    << "void main()" << std::endl
                    << "{" << std::endl
                    << "    vec4 col = texture2D(img, gl_TexCoord[0].st);" << std::endl
                    << "    gl_FragColor = " << shaderDesc->getFunctionName() << "(col);" << std::endl
                    << "}" << std::endl;

                this->m_OGLBuilder->buildProgram(frag.str().c_str());
                this->m_OGLBuilder->useProgram();
                glUniform1i(glGetUniformLocation(this->m_OGLBuilder->getProgramHandle(), "img"), 0);
                this->m_OGLBuilder->useAllTextures();
                this->m_OGLBuilder->useAllUniforms();
            }
            else
            // use the cpu processor
            {
                this->m_CPU = processor->getOptimizedCPUProcessor(OCIO::OPTIMIZATION_ALL);
            }
        }
        catch (OCIO::Exception& exception)
        {
            this->m_Logger->Log(LogLevel_Warning, "[OCIO] : %s", exception.what());

            const char* display = this->m_Config->getDefaultDisplay();
            const char* view = this->m_Config->getDefaultView(display);

            OCIO::ConstProcessorRcPtr processor = this->m_Config->getProcessor(OCIO::ROLE_SCENE_LINEAR, display, view, OCIO::TRANSFORM_DIR_FORWARD);
            
            // if we use the gpu, create and compile the shader needed for the transform
            if (this->m_UseGPU)
            {
                OCIO::GpuShaderDescRcPtr shaderDesc;
                GL_CHECK(shaderDesc = OCIO::GpuShaderDesc::CreateShaderDesc());
                shaderDesc->setLanguage(OCIO::GPU_LANGUAGE_GLSL_1_3);
                shaderDesc->setFunctionName("OCIODisplay");
                shaderDesc->setResourcePrefix("ocio_");

                this->m_GPU = processor->getOptimizedGPUProcessor(OCIO::OPTIMIZATION_ALL);

                this->m_GPU->extractGpuShaderInfo(shaderDesc);

                this->m_OGLBuilder = OCIO::OpenGLBuilder::Create(shaderDesc);

                this->m_OGLBuilder->allocateAllTextures(1);

                std::ostringstream frag;
                frag << std::endl
                    << "uniform sampler2D img;" << std::endl
                    << std::endl
                    << "void main()" << std::endl
                    << "{" << std::endl
                    << "    vec4 col = texture2D(img, gl_TexCoord[0].st);" << std::endl
                    << "    gl_FragColor = " << shaderDesc->getFunctionName() << "(col);" << std::endl
                    << "}" << std::endl;

                this->m_OGLBuilder->buildProgram(frag.str().c_str());
                this->m_OGLBuilder->useProgram();
                glUniform1i(glGetUniformLocation(m_OGLBuilder->getProgramHandle(), "img"), 0);
                this->m_OGLBuilder->useAllTextures();
                this->m_OGLBuilder->useAllUniforms();
            }
            else
            // use the cpu processor, much slower
            {
                this->m_CPU = processor->getOptimizedCPUProcessor(OCIO::OPTIMIZATION_ALL);
            }
        }
    }

    void Ocio::Process(const uint16_t width, const uint16_t height)
    {
        this->m_OGLBuilder->useProgram();
        glUniform1i(glGetUniformLocation(this->m_OGLBuilder->getProgramHandle(), "img"), 0);
        this->m_OGLBuilder->useAllTextures();
        this->m_OGLBuilder->useAllUniforms();
    }

    // cleanup the vectors containing views, displays, roles and looks
    void Ocio::Release() noexcept
    {
        this->m_Views.clear();
        this->m_Displays.clear();
        this->m_Roles.clear();
        this->m_Looks.clear();

        m_Logger->Log(LogLevel_Diagnostic, "[OCIO] : Released OCIO");
    }
} // End namespace Core