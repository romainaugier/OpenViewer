// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include "imgui.h"

#include "core/loader.h"
#include "app.h"

namespace Interface
{
    struct MediaExplorer
    {
        Core::Loader* m_Loader = nullptr;

        Logger* m_Logger = nullptr;

        ImVec2 m_CurrentMediaRange = ImVec2(0, 0);

        bool m_CurrentMediaChanged = false;

        MediaExplorer(Core::Loader* loader, Logger* logger);

        void Draw(Application* app, bool& showWindow) noexcept;
    };
}