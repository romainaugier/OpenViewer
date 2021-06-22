// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#pragma once

#include "vector"
#include "stdio.h"
#include "iostream"
#include "OpenColorIO.h"
#include "GL/gl3w.h"
#include "oglapp.h"

#include "utils/string_utils.h"

namespace OCIO = OCIO_NAMESPACE;

struct Ocio
{
    OCIO::ConstConfigRcPtr config;
    std::vector<char*> active_views;
    const char* current_view;
    const char* current_display;

    void Initialize();
    void GetOcioActiveViews() noexcept;
    void ChangeConfig(const char* config_path);
    void Process(float* __restrict buffer, const uint16_t width, const uint16_t height);
};