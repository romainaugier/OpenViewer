// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include <GL/glew.h>
#include "utils/gl_utils.h"
#include "utils/logger.h"
#include "utils/filesystem_utils.h"

#undef max
#undef min

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <cmath>
#ifdef __GNUC__
#include <cstring>
#endif
#include "OpenColorIOv2/OpenColorIO.h"
#include "glsl.h"

#include "utils/string_utils.h"
#include "utils/memory/track.h"

namespace OCIO = OCIO_NAMESPACE;

namespace Core
{
    struct Ocio
    {
        OCIO::ConstConfigRcPtr m_Config;
        
        OCIO::OpenGLBuilderRcPtr m_OGLBuilder;
        
        OCIO::ConstCPUProcessorRcPtr m_CPU;
        OCIO::ConstGPUProcessorRcPtr m_GPU;

        std::vector<std::string> m_Views;
        std::vector<std::string> m_Displays;
        std::vector<std::string> m_Roles;
        std::vector<std::string> m_Looks;
        
        std::string m_ConfigPath;
        std::string m_CurrentView;
        std::string m_CurrentDisplay;
        std::string m_CurrentRole;
        std::string m_CurrentLook;
        
        Logger* m_Logger;
        
        int m_ChannelsHot[4] = { 1, 1, 1, 1 };
        int m_CurrentChannelIdx = 0;
        int m_CurrentViewIdx = 0;
        int m_CurrentDisplayIdx = 0;
        int m_CurrentRoleIdx = 0;
        int m_CurrentLookIdx = 0;
        
        float m_ExposureStops = 0.0f;
        float m_Gamma = 1.0f;
        
        bool m_UseGPU = true;

        Ocio(Logger* log)
        {
            m_Logger = log;
        }

        void Initialize();

        void GetOcioActiveViews() noexcept;

        void GetOcioDisplayViews() noexcept;

        void GetOcioActiveDisplays() noexcept;

        void GetRoles() noexcept;

        void GetLooks() noexcept;

        void ChangeConfig(const char* configPath);

        void UpdateChannelHot() noexcept;

        void UpdateProcessor();

        void Process(const uint16_t width, const uint16_t height);

        void Release() noexcept;
    };
} // End namespace Core