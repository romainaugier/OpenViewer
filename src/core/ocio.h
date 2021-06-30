// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include "vector"
#include "stdio.h"
#include "iostream"
#include "OpenColorIO.h"

#include "utils/string_utils.h"

namespace OCIO = OCIO_NAMESPACE;

struct Ocio
{
    OCIO::ConstConfigRcPtr config;
    OCIO::ConstCPUProcessorRcPtr cpu;
    std::vector<std::string> active_views;
    std::vector<std::string> active_displays;
    const char* current_view = nullptr;
    const char* current_display = nullptr;

    void Initialize();
    void GetOcioActiveViews() noexcept;
    void GetOcioActiveDisplays() noexcept;
    void ChangeConfig(const char* config_path);
    void UpdateProcessor() noexcept;
    void Process(float* const __restrict buffer, const uint16_t width, const uint16_t height);
};