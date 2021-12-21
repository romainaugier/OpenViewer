// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include <unordered_map>

#include "utils/logger.h"
#include "display.h"

namespace Interface
{
    using Displays = std::unordered_map<uint8_t, Display*>;
    using WindowsToShow = std::unordered_map<std::string, bool>;

    struct Application
    {
        Displays m_Displays;
        WindowsToShow m_Windows;

        Logger* m_Logger = nullptr;

        uint8_t m_ActiveDisplayID = 0;
        uint8_t m_DisplayCount = 0;

        Application(Logger* logger);        
    };
}