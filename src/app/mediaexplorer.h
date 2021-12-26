// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include "imgui.h"

#include "core/loader.h"

namespace Interface
{
    struct MediaExplorer
    {
        Core::Loader* m_Loader = nullptr;

        MediaExplorer(Core::Loader* loader);

        void Draw(bool& showWindow) noexcept;
    };
}