// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 Romain Augier
// All rights reserved.

#pragma once

#include "imgui.h"
#include "tsl/robin_map.h"

#include "media.h"

namespace Core
{
    class Timeline
    {
    public:
        Timeline() {}

        ~Timeline() {}

        void Initialize(const ImVec2 range) noexcept;

        void Add(Media* media) noexcept;

        void Remove(Media* media) noexcept;

        // Getters / Setters
        OV_FORCEINLINE Media* GetMediaAtFrame(const uint32_t frame) noexcept;
        OV_FORCEINLINE void SetRange(const ImVec2 newRange) noexcept { this->m_Range = newRange; }
        OV_FORCEINLINE ImVec2 GetRange() const noexcept { return this->m_Range; }
        
        void Release() noexcept;

    private:
        tsl::robin_map<std::string, Media*> m_Medias;

        ImVec2 m_Range;
    };
}