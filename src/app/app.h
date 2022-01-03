// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include "tsl/robin_map.h"

#include "utils/logger.h"
#include "display.h"

namespace Interface
{
    using Displays = tsl::robin_map<uint8_t, Display*>;

    struct Application
    {
        Displays m_Displays;

        Core::Loader* m_Loader = nullptr;
        Core::Ocio* m_OcioModule = nullptr;
        Logger* m_Logger = nullptr;

        uint8_t m_ActiveDisplayID = 0;
        uint8_t m_DisplayCount = 0;

        // Bools to control the state of the different windows
        bool showImageInfosWindow = false;
        bool showPixelInfosWindow = false;
        bool showMediaInfosWindow = false;
        bool showMediaExplorerWindow = false;

        Application(Logger* logger, Core::Loader* loader, Core::Ocio* ocio);     

        void Release() noexcept;   
    };
}