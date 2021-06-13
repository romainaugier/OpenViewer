// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#pragma once

#include "vector"
#include "stdio.h"
#include "iostream"
#include "OpenColorIO/OpenColorIO.h"

namespace OCIO = OCIO_NAMESPACE;

struct Ocio
{
    OCIO::ConstConfigRcPtr config;
    std::vector<char*> active_views;
    char* current_view;
    char* current_display;

    void Initialize();
    void GetOcioActiveViews();
};