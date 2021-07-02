// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include "vector"
#include "stdio.h"
#include "iostream"
#include "OpenColorIOv2/OpenColorIO.h"

#include "utils/string_utils.h"

namespace OCIO = OCIO_NAMESPACE;

struct Ocio
{
    OCIO::ConstConfigRcPtr config;
    OCIO::ConstCPUProcessorRcPtr cpu;
    std::vector<const char*> active_views;
    std::vector<const char*> active_displays;
    std::vector<const char*> roles;
    const char* current_view = nullptr;
    int current_view_idx = 0;
    const char* current_display = nullptr;
    int current_display_idx = 0;
    const char* current_role = nullptr;
    int current_role_idx = 0;

    void Initialize();
    void GetOcioActiveViews() noexcept;
    void GetOcioDisplayViews() noexcept;
    void GetOcioActiveDisplays() noexcept;
    void GetRoles() noexcept;
    void ChangeConfig(const char* config_path);
    void UpdateProcessor();
    void Process(float* const __restrict buffer, const uint16_t width, const uint16_t height);
    void Release() noexcept;
};