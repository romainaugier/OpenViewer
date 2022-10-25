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
        Imf::MultiPartInputFile multiPartInputFile(this->m_Path.c_str());

        this->m_IsMultipart = multiPartInputFile.parts() > 1;

        uint16_t layerCount = 0;

        // Simple and dirty method to retrieve the basic rgb/rgba/z channels and approximate the number of layers
        uint8_t rgbOrRgbaChannels = 0;

        bool rChannel = false; bool gChannel = false; bool bChannel = false;

        const std::regex singleRChannelPattern("^R$");
        const std::regex singleGChannelPattern("^G$");
        const std::regex singleBChannelPattern("^B$");
        const std::regex singleAChannelPattern("^A$");
        const std::regex zDepthChannelPattern("^Z$|^Z\\.|Z_1|^z$|^z\\.|z_1");

        for (uint32_t i = 0; i < multiPartInputFile.parts(); i++)
        {
            for (auto f = multiPartInputFile.header(i).channels().begin(); f != multiPartInputFile.header(i).channels().end(); ++f)
            {   
                if (multiPartInputFile.header(i).name().size() == 0)
                {
                    if (std::regex_search(f.name(), singleRChannelPattern)) { ++rgbOrRgbaChannels; rChannel = true; printf("R\n"); }
                    else if (std::regex_search(f.name(), singleGChannelPattern)) { ++rgbOrRgbaChannels; gChannel = true; printf("G\n"); }
                    else if (std::regex_search(f.name(), singleBChannelPattern)) { ++rgbOrRgbaChannels; bChannel = true; printf("B\n"); }
                    else if (std::regex_search(f.name(), singleAChannelPattern)) { ++rgbOrRgbaChannels; printf("A\n"); }
                    else if (std::regex_search(f.name(), zDepthChannelPattern)) this->m_Layers.emplace_back(std::make_pair(f.name(), f.name()));
                }
                ++layerCount;
            }
        }

        layerCount /= 4;
        this->m_Layers.reserve(layerCount);

        if (rgbOrRgbaChannels == 3)
        {
            this->m_Layers.insert(this->m_Layers.begin(), std::make_pair("Beauty", "R;G;B"));
        }
        else if (rgbOrRgbaChannels == 4)
        {
            this->m_Layers.insert(this->m_Layers.begin(), std::make_pair("Beauty", "R;G;B;A"));
        }
        else if (rChannel && !gChannel && !bChannel) this->m_Layers.insert(this->m_Layers.begin(), std::make_pair("Beauty", "R"));
        else if (!rChannel && gChannel && !bChannel) this->m_Layers.insert(this->m_Layers.begin(), std::make_pair("Beauty", "G"));
        else if (!rChannel && !gChannel && bChannel) this->m_Layers.insert(this->m_Layers.begin(), std::make_pair("Beauty", "B"));

        const std::regex rChannelPattern("\\.R|^R$|\\.R\\.");
        const std::regex gChannelPattern("\\.G|^G$|\\.G\\.");
        const std::regex bChannelPattern("\\.B|^B$|\\.B\\.");
        const std::regex aChannelPattern("\\.A|^A$|\\.A\\.");
        const std::regex xChannelPattern("\\.X|^X$|\\.X\\.");
        const std::regex yChannelPattern("\\.Y|^Y$|\\.Y\\.");
        const std::regex zChannelPattern("\\.Z|^Z$|\\.Z\\.");

        for (uint32_t i = 0; i < multiPartInputFile.parts(); i++)
        {
            std::set<std::string> layers;

            multiPartInputFile.header(i).channels().layers(layers);

            std::string channelPrefix;

            if (multiPartInputFile.header(i).hasName())
            {
                channelPrefix += multiPartInputFile.header(i).name();
                channelPrefix += ".";
            }
            else
            {
                continue;
            }

            if (multiPartInputFile.header(i).hasView())
            {

            }

            for (const auto& layer : layers)
            {
                Imf::ChannelList::ConstIterator layerBegin, layerEnd;

                multiPartInputFile.header(i).channels().channelsInLayer(layer, layerBegin, layerEnd);

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