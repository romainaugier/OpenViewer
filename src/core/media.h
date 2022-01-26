// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include <vector>

#include "image.h"

// WinUser.h has a LoadImage definition
#ifdef _WIN32
#undef LoadImage
#endif

#include "imgui.h"

// A media holds the images and the range/length of the media
// If the media is a single image, the range will be 0-0

namespace Core
{
    class Media
    {
    public:
        Media() {}

        Media(const std::string& path, const uint32_t id);

        virtual ~Media() {}

        // Loads an image to the given address
        virtual void LoadImage(const uint32_t frameIndex, void* buffer) noexcept = 0;

        // Returns an image corresponding to the frame. If the frameIndex is out of range,
        // returns nullptr
        virtual Image* GetImage(const uint32_t frameIndex) noexcept = 0;

        virtual void Release() noexcept = 0;
        
        // Getters/Setters
        OV_FORCEINLINE void SetImages(const std::vector<Image>& images) noexcept { this->m_Images = std::move(images); }
        OV_FORCEINLINE void SetID(const uint32_t id) noexcept { this->m_ID = id; }
        OV_FORCEINLINE uint32_t ID() const noexcept { return this->m_ID; }
        OV_FORCEINLINE uint32_t Size() const noexcept { return static_cast<uint32_t>(this->m_Range.y); }
        OV_FORCEINLINE std::string Path() const noexcept { return this->m_Path; }
        OV_FORCEINLINE void SetRange(const ImVec2& range) noexcept { this->m_Range = range; }
        OV_FORCEINLINE ImVec2 GetRange() const noexcept { return this->m_Range; }
        OV_FORCEINLINE void SetTimelineRange(const ImVec2& range) noexcept { this->m_TimelineRange = range; }
        OV_FORCEINLINE ImVec2 GetTimelineRange() const noexcept { return this->m_TimelineRange; }
        OV_FORCEINLINE void SetTotalByteSize(const uint64_t& size) noexcept { this->m_TotalByteSize = size; }
        OV_FORCEINLINE uint64_t GetTotalByteSize() const noexcept { return this->m_TotalByteSize; }
        OV_FORCEINLINE void SetBiggestImageSize(const uint64_t& size) noexcept { this->m_BiggestImageSize = size; }
        OV_FORCEINLINE uint64_t GetBiggestImageSize() const noexcept { return this->m_BiggestImageSize; }

    protected:
        std::vector<Image> m_Images;

        std::string m_Path;

        uint32_t m_ID;
        
        bool InRange(const uint32_t index) const noexcept; 

    private:
        uint64_t m_TotalByteSize = 0;
        uint64_t m_BiggestImageSize = 0;

        ImVec2 m_Range;
        ImVec2 m_TimelineRange = ImVec2(0, 0);
    };

    // Class that holds an image sequence
    class ImageSequence : public Media
    {
    public:
        ImageSequence(const std::string& path, const uint32_t id);

        void LoadImage(const uint32_t frameIndex, void* buffer) noexcept override;

        Image* GetImage(const uint32_t frameIndex) noexcept override;

        void Release() noexcept override;
    };

    // Special case for exr has they have layers
    
    // A layer represents an exr part, with channels. The first string in the pair
    // holds the "general" name, and the second one holds the channels names separated by a ;
    using Layer = std::pair<std::string, std::string>;

    class EXRSequence : public Media
    {
    public:
        int m_CurrentLayerID = 0;

        EXRSequence(const std::string& path, const uint32_t id);

        void LoadImage(const uint32_t frameIndex, void* buffer) noexcept override;

        Image* GetImage(const uint32_t frameIndex) noexcept override;

        void Release() noexcept override;

        void SetLayers() noexcept;

        // Getters/Setters
        OV_FORCEINLINE std::string GetCurrentChannels() const noexcept { return this->m_Layers[this->m_CurrentLayerID].second; }
        OV_FORCEINLINE std::string GetCurrentLayerName() const noexcept { return this->m_CurrentLayer; }
        OV_FORCEINLINE std::vector<Layer> GetLayers() const noexcept { return this->m_Layers; }
        OV_FORCEINLINE void UpdateCurrentLayer() noexcept { this->m_CurrentLayer = this->m_Layers[this->m_CurrentLayerID].first; }
        OV_FORCEINLINE void SetNumThreads(const uint8_t numThreads) noexcept { this->m_NumThreads = numThreads; }

    private:
        std::vector<Layer> m_Layers;

        std::string m_CurrentLayer = "Beauty";

        uint8_t m_NumThreads = 8;

        bool m_IsMultipart = false;
    };

    // Class that holds a video stream
    class Video : public Media
    {
    public:
        Video(const std::string& path, const uint32_t id);

        void LoadImage(const uint32_t frameIndex, void* buffer) noexcept override;

        Image* GetImage(const uint32_t frameIndex) noexcept override;
        
        void Release() noexcept override;
    };
}