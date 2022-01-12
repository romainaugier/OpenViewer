// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "media.h"

namespace Core
{
    uint32_t Media::Size() const noexcept
    {
        return this->m_Range.y;
    }

    bool Media::InRange(const uint32_t index) const noexcept
    {
        return index >= this->m_TimelineRange.x && index < this->m_TimelineRange.y;
    }

    void Media::SetActive() noexcept
    {
        this->m_IsActive = true;
        this->m_TimelineRange = this->m_Range;
    }
    
    void Media::SetInactive() noexcept
    {
        this->m_IsActive = false;
        this->m_TimelineRange = ImVec2(0, 0);
    }

    void Media::SetLayers() noexcept
    {
        const Image firstImage = this->m_Images[0];

        auto in = OIIO::ImageInput::open(firstImage.m_Path);
        const OIIO::ImageSpec& spec = in->spec();
        const uint16_t channelsCount = spec.channelnames.size();

        const std::regex channelNamePattern("\\.R|^R$|\\.G|^G$|\\.B|^B$|\\.A|^A$|\\.X|\\.Y|\\.Z");

        this->m_Layers.reserve(channelsCount);

        std::string previousLayerName = "Beauty";
        std::string layerName = "";

        std::string layerNames = "";

        for (uint16_t i = 0; i < (channelsCount + 1); i++)
        {
            if (i == channelsCount) layerName = "lastvirtuallayer";
            else layerName = spec.channelnames[i];
            
            std::string layerNameClean = layerName;

            Utils::Str::ReReplace(layerNameClean, channelNamePattern, "");

            if (layerNameClean == "") layerNameClean = "Beauty";

            if (!(layerNameClean == previousLayerName))
            {
                this->m_Layers.emplace_back(std::make_pair(previousLayerName, layerNames));
                
                previousLayerName = layerNameClean;

                layerNames = layerName;
            }
            else
            {
                layerNames += layerNames != "" ? (";" + layerName) : layerName;
                previousLayerName = layerNameClean;
            }
        }

        this->m_Layers.shrink_to_fit();
    }
}