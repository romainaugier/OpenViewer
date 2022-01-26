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

    void ImageSequence::Release() noexcept
    {

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
        Imf::MultiPartInputFile file(this->m_Path.c_str());

        this->m_IsMultipart = file.parts() > 1;

        uint16_t layerCount = 0;

        // Simple and dirty method to retrieve the basic rgb/rgba channels and approximate the number of layers
        uint8_t rgbOrRgbaChannels = 0;

        const std::regex singleRChannelPattern("^R$");
        const std::regex singleGChannelPattern("^G$");
        const std::regex singleBChannelPattern("^B$");
        const std::regex singleAChannelPattern("^A$");

        for (uint32_t i = 0; i < file.parts(); i++)
        {
            for (auto f = file.header(i).channels().begin(); f != file.header(i).channels().end(); ++f)
            {
                if (std::regex_search(f.name(), singleRChannelPattern)) ++rgbOrRgbaChannels;
                else if (std::regex_search(f.name(), singleGChannelPattern)) ++rgbOrRgbaChannels;
                else if (std::regex_search(f.name(), singleBChannelPattern)) ++rgbOrRgbaChannels;
                else if (std::regex_search(f.name(), singleAChannelPattern)) ++rgbOrRgbaChannels;

                ++layerCount;
            }
        }

        layerCount /= 4;
        this->m_Layers.reserve(layerCount);

        if (rgbOrRgbaChannels == 3)
        {
            this->m_Layers.emplace_back(std::make_pair("Beauty", "R;G;B"));
        }
        else if (rgbOrRgbaChannels == 4)
        {
            this->m_Layers.emplace_back(std::make_pair("Beauty", "R;G;B;A"));
        }

        const std::regex rChannelPattern("\\.R|^R$|\\.R\\.");
        const std::regex gChannelPattern("\\.G|^G$|\\.G\\.");
        const std::regex bChannelPattern("\\.B|^B$|\\.B\\.");
        const std::regex aChannelPattern("\\.A|^A$|\\.A\\.");
        const std::regex xChannelPattern("\\.X|^X$|\\.X\\.");
        const std::regex yChannelPattern("\\.Y|^Y$|\\.Y\\.");
        const std::regex zChannelPattern("\\.Z|^Z$|\\.Z\\.");

        for (uint32_t i = 0; i < file.parts(); i++)
        {
            std::set<std::string> layers;

            file.header(i).channels().layers(layers);

            std::string channelPrefix;

            if (file.header(i).hasName())
            {
                channelPrefix += file.header(i).name();
                channelPrefix += ".";
            }

            if (file.header(i).hasView())
            {

            }

            for (const auto& layer : layers)
            {
                Imf::ChannelList::ConstIterator layerBegin, layerEnd;

                file.header(i).channels().channelsInLayer(layer, layerBegin, layerEnd);

                std::vector<std::string> channelNamesSorted; channelNamesSorted.resize(4);

                for (Imf::ChannelList::ConstIterator j = layerBegin; j != layerEnd; ++j)
                {
                    const std::string channelName = j.name();

                    // A bit dirty channel names sorting
                    if (std::regex_search(channelName, rChannelPattern))
                    {
                        channelNamesSorted[0] = channelName;
                    }
                    else if (std::regex_search(channelName, gChannelPattern))
                    {
                        channelNamesSorted[1] = channelName;
                    }
                    else if (std::regex_search(channelName, bChannelPattern))
                    {
                        channelNamesSorted[2] = channelName;
                    }
                    else if (std::regex_search(channelName, aChannelPattern))
                    {
                        channelNamesSorted[3] = channelName;
                    }
                    else if (std::regex_search(channelName, xChannelPattern))
                    {
                        channelNamesSorted[0] = channelName;
                    }
                    else if (std::regex_search(channelName, yChannelPattern))
                    {
                        channelNamesSorted[1] = channelName;
                    }
                    else if (std::regex_search(channelName, zChannelPattern))
                    {
                        channelNamesSorted[2] = channelName;
                    }
                }

                std::string channelNamesConcat;

                int i = 0;

                for (const auto& channelName : channelNamesSorted)
                {
                    if (channelName != "") channelNamesConcat += i == 0 ? channelName : ";" + channelName;

                    ++i;
                }

                this->m_Layers.emplace_back(std::make_pair(layer, channelNamesConcat));
            }
        }

        this->m_Layers.shrink_to_fit();
    }

    void EXRSequence::Release() noexcept
    {

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

    void Video::Release() noexcept
    {
        
    }
}