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
    // A layer represents an exr part, with channels. The first string in the pair
    // holds the "general" name, and the second one holds the channels names separated by a ;
    using Layer = std::pair<std::string, std::string>;
    struct Media
    {
        std::vector<Image> m_Images; // Holds the images of the sequence 

        std::vector<Layer> m_Layers; // Holds the layer names if the image has any

        std::string m_CurrentLayerStr = "Beauty";

        ImVec2 m_Range = ImVec2(0, 0); // "Raw" range of the sequence
        ImVec2 m_TimelineRange = ImVec2(0, 0); // Range in the timeline

        uint32_t m_ID;

        int32_t m_CurrentLayerID = 0;

        bool m_IsActive = false; // The media is in the timeline, and so is/will be displayed

        uint32_t Size() const noexcept;

        bool InRange(const uint32_t index) const noexcept; 

        void SetActive() noexcept;
        
        void SetInactive() noexcept;
        
        void SetLayers() noexcept;

        bool HasLayers() const noexcept { return this->m_Layers.size() > 0; }

        std::string GetCurrentChannels() const noexcept { return this->m_Layers[this->m_CurrentLayerID].second; }

        void UpdateCurrentLayer() noexcept { this->m_CurrentLayerStr = this->m_Layers[this->m_CurrentLayerID].first; }
    };
}