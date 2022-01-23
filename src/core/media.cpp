// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "media.h"

namespace Core
{
    Media::Media(const std::string& path, const uint32_t id)
    {
        this->m_Path = path;
        this->m_ID = id;
    }

    bool Media::InRange(const uint32_t index) const noexcept
    {
        return index >= this->m_Range.x && index < this->m_Range.y;
    }

    ImageSequence::ImageSequence(const std::string& path, const uint32_t id)
    {
        this->m_Path = path;
        this->m_ID = id;
    }

    void ImageSequence::LoadImage(const uint32_t frameIndex, void* buffer) noexcept 
    {
        assert(this->InRange(frameIndex));

        this->m_Images[frameIndex].Load(buffer);
    }

    Image* ImageSequence::GetImage(const uint32_t frameIndex) noexcept 
    {
        assert(this->InRange(frameIndex));

        return &this->m_Images[frameIndex];
    }

    EXRSequence::EXRSequence(const std::string& path, const uint32_t id)
    {
        this->m_Path = path;
        this->m_ID = id;
    }

    void EXRSequence::LoadImage(const uint32_t frameIndex, void* buffer) noexcept 
    {
        assert(this->InRange(frameIndex));

        this->m_Images[frameIndex].LoadExr(buffer, 
                                           this->m_Layers[this->m_CurrentLayerID].second,
                                           this->m_NumThreads);
    }

    Image* EXRSequence::GetImage(const uint32_t frameIndex) noexcept 
    {
        assert(this->InRange(frameIndex));

        return &this->m_Images[frameIndex];
    }

    void EXRSequence::SetLayers() noexcept
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

    Video::Video(const std::string& path, const uint32_t id)
    {
        this->m_Path = path;
        this->m_ID = id;
    }

    void Video::LoadImage(const uint32_t frameIndex, void* buffer) noexcept 
    {

    }

    Image* Video::GetImage(const uint32_t frameIndex) noexcept 
    {
        printf("video\n");
        return nullptr;
    }
}