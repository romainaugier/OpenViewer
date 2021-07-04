// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include <GL/glew.h>
#include <GL/glut.h>
#include "utils/gl_utils.h"

#undef max
#undef min

#include <vector>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <cmath>
#include "OpenColorIOv2/OpenColorIO.h"
#include "glsl.h"

#include "utils/string_utils.h"

namespace OCIO = OCIO_NAMESPACE;

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
    const char* current_view = nullptr;
    const char* current_display = nullptr;
    const char* current_role = nullptr;
    const char* current_look = nullptr;
    int channel_hot[4] = { 1, 1, 1, 1 };
    int current_channel_idx = 0;
    int current_view_idx = 0;
    int current_display_idx = 0;
    int current_role_idx = 0;
    int current_look_idx = 0;
    float exposure_stops = 0.0f;
    float gamma = 1.0f;
    unsigned int use_gpu : 1;

    Ocio()
    {
        use_gpu = 1;
    }

    void Initialize();
    void GetOcioActiveViews() noexcept;
    void GetOcioDisplayViews() noexcept;
    void GetOcioActiveDisplays() noexcept;
    void GetRoles() noexcept;
    void GetLooks() noexcept;
    void ChangeConfig(const char* config_path);
    void UpdateProcessor();
    void Process(float* const __restrict buffer, const uint16_t width, const uint16_t height);
    void Release() noexcept;
};