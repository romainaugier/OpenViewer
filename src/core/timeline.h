// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 Romain Augier
// All rights reserved.

#pragma once

#include "imgui.h"
#include "tsl/robin_map.h"

#include "media.h"

namespace Core
{
    // A sequence is just a little container to represent a media in the timeline of OpenViewer
    struct Sequence
    {
        Media* m_Media;

        ImVec2 m_Range;

        uint32_t m_Offset; // When the sequence is cut, offset from the beginning of the sequence
    };

    // The timeline is the organisation of the different sequences and how to play them (to make editing/montage)
    class Timeline
    {
    public:
        Timeline() {}

        ~Timeline() {}

        void Initialize(const ImVec2& range) noexcept;

        void Add(Media* media) noexcept;

        void Remove(Media* media) noexcept;

        // Getters / Setters
        OV_FORCEINLINE Media* GetMediaAtFrame(const uint32_t frame) noexcept;
        OV_FORCEINLINE void SetRange(const ImVec2 newRange) noexcept { this->m_Range = newRange; }
        OV_FORCEINLINE ImVec2 GetRange() const noexcept { return this->m_Range; }
        
        void Release() noexcept;

    private:
        tsl::robin_map<std::string, Sequence*> m_Sequences;

        ImVec2 m_Range;
    };
}