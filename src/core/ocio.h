// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include <GL/glew.h>
#include "utils/gl_utils.h"
#include "utils/logger.h"

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
        OCIO::ConstConfigRcPtr config;
        OCIO::OpenGLBuilderRcPtr ogl_builder;
        OCIO::ConstCPUProcessorRcPtr cpu;
        OCIO::ConstGPUProcessorRcPtr gpu;
        std::vector<const char*> views;
        std::vector<const char*> displays;
        std::vector<const char*> roles;
        std::vector<const char*> looks;
        const char* config_path = nullptr;
        const char* current_view = nullptr;
        const char* current_display = nullptr;
        const char* current_role = nullptr;
        const char* current_look = nullptr;
        Logger* logger;
        int channel_hot[4] = { 1, 1, 1, 1 };
        int current_channel_idx = 0;
        int current_view_idx = 0;
        int current_display_idx = 0;
        int current_role_idx = 0;
        int current_look_idx = 0;
        float exposure_stops = 0.0f;
        float gamma = 1.0f;
        unsigned int use_gpu : 1;

        Ocio(Logger* log)
        {
            use_gpu = 1;
            logger = log;
        }

        uint32_t GetSize() const noexcept;
        void Initialize();
        void GetOcioActiveViews() noexcept;
        void GetOcioDisplayViews() noexcept;
        void GetOcioActiveDisplays() noexcept;
        void GetRoles() noexcept;
        void GetLooks() noexcept;
        void ChangeConfig(const char* config_path);
        void UpdateProcessor();
        void Process(const uint16_t width, const uint16_t height);
        void Release() noexcept;
    };
} // End namespace Core