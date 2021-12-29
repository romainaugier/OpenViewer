// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include "core/image.h"
#include "display.h"

#include "imgui.h"

namespace Interface
{
    struct ImageInfo
    {   
        void Draw(const Core::Image& currentImage, bool& showWindow) const noexcept;
    };

    struct PixelInfo
    {
        void Draw(Core::Loader* loader, const Core::Image& currentImage, Display* currentDisplay, bool& showWindow) const noexcept;
    };
}