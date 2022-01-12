// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include "tsl/robin_map.h"

#include "utils/logger.h"
#include "display.h"
#include "shortcuts.h"
#include "implaybar.h"
#include "menubar.h"
#include "scopes.h"

namespace Interface
{
    using Displays = tsl::robin_map<uint8_t, std::pair<bool, Display*>>;

    struct Menubar;

    struct Application
    {
        Displays m_Displays;

        Shortcuts m_Shortcuts;

        Core::Loader* m_Loader = nullptr;
        Core::Ocio* m_OcioModule = nullptr;
        Logger* m_Logger = nullptr;
        ImPlaybar* m_Playbar = nullptr;
        Menubar* m_Menubar = nullptr;

        uint8_t m_ActiveDisplayID = 0;
        uint8_t m_DisplayCount = 0;

        bool m_ChangeEvent = false;

        // Bools to control the state of the different windows
        bool showImageInfosWindow = false;
        bool showPixelInfosWindow = false;
        bool showMediaInfosWindow = false;
        bool showMediaExplorerWindow = false;

        Application(Logger* logger, Core::Loader* loader, Core::Ocio* ocio);    
        
        // Few setters and getters
        void SetPlaybar(ImPlaybar* playbar) noexcept { this->m_Playbar = playbar; } 
        void SetMenubar(Menubar* menubar) noexcept { this->m_Menubar = menubar; }

        void Changed() noexcept { this->m_ChangeEvent = true; }
        bool SomethingChanged() noexcept { return this->m_ChangeEvent; }
        void ClearChange() noexcept { this->m_ChangeEvent = false; }

        Display* GetActiveDisplay() noexcept;

        // Windows openers
        void ShowImageInfosWindow() noexcept { this->showImageInfosWindow = !this->showImageInfosWindow; }
        void ShowPixelInfosWindow() noexcept { this->showPixelInfosWindow = !this->showPixelInfosWindow; }
        void ShowMediaInfosWindow() noexcept { this->showMediaInfosWindow = !this->showMediaInfosWindow; }
        void ShowMediaExplorerWindow() noexcept { this->showMediaExplorerWindow = !this->showMediaExplorerWindow; }

        // Shortcut handling
        void HandleShortcuts() noexcept;

        // Function that updates displays states
        void UpdateDisplays() noexcept;

        void Release() noexcept;   
    };
}