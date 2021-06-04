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