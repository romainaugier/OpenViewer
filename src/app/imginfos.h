// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include "core/image.h"

#include "imgui.h"

namespace Interface
{
    struct ImageInfo
    {   
        void Draw(const Core::Image& currentImage, bool& showWindow) const noexcept;
    };
}