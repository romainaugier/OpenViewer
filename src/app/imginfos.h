// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include "core/media.h"
#include "display.h"

#include "imgui.h"

namespace Interface
{
    struct ImageInfo
    {   
        void Draw(const Core::Image& currentImage, const Core::Media* currentMedia, bool& showWindow) const noexcept;
    };

    struct PixelInfo
    {
        void Draw(const Core::Loader* loader, const Core::Image& currentImage, const Display* currentDisplay, bool& showWindow) const noexcept;
    };
}