// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include <vector>

#include "image.h"

#include "imgui.h"

// A media holds the images and the range/length of the media
// If the media is a single image, the range will be 0-0

namespace Core
{
    struct Media
    {
        std::vector<Image> m_Images; // Holds the images of the sequence 

        ImVec2 m_Range = ImVec2(0, 0); // "Raw" range of the sequence
        ImVec2 m_TimelineRange = ImVec2(0, 0); // Range in the timeline

        bool m_IsActive = false; // The media is in the timeline, and so is/will be displayed

        uint32_t Size() const noexcept;
        bool InRange(const uint32_t index) const noexcept; 
    };
}