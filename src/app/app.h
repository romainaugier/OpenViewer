// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include <unordered_map>

#include "utils/logger.h"
#include "display.h"

namespace Interface
{
    using Displays = std::unordered_map<uint8_t, Display*>;

    struct Application
    {
        Displays m_Displays;

        Core::Loader* m_Loader = nullptr;
        Logger* m_Logger = nullptr;

        uint8_t m_ActiveDisplayID = 0;
        uint8_t m_DisplayCount = 0;

        // Bool to control the state of the different windows
        bool showImageInfosWindow = false;
        bool showMediaInfosWindow = false;
        bool showMediaExplorerWindow = false;

        Application(Logger* logger, Core::Loader* loader);     

        void Release() noexcept;   
    };
}