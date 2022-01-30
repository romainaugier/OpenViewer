// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 Romain Augier
// All rights reserved.

#pragma once

#include "imgui.h"
#include "tsl/robin_map.h"

#include "loader.h"

namespace Core
{
    // A sequence is just a little container to represent a media in the timeline of OpenViewer
    struct Sequence
    {
        Media* m_Media;

        ImVec2 m_Range;

        uint32_t m_Offset; // When the sequence is cut, offset from the beginning of the sequence

        OV_FORCEINLINE void SetRange(const ImVec2& newRange) noexcept { this->m_Range = newRange; }
        OV_FORCEINLINE ImVec2 GetRange() const noexcept { return this->m_Range; }
    };

    // The timeline is the organisation of the different sequences and how to play them (to make editing/montage).
    // It also provides a playback system
    class Timeline
    {
    public:
        Timeline() {}

        ~Timeline() {}

        void Initialize(const ImVec2& range, Logger* logger, Loader* loader) noexcept;

        void Add(Media* media) noexcept;

        void Remove(const uint32_t mediaId) noexcept;

        void Update() noexcept;

        // Getters / Setters
        Media* GetMediaAtFrame(const uint32_t frame) noexcept;
        OV_FORCEINLINE void SetRange(const ImVec2& newRange) noexcept { this->m_Range = newRange; }
        OV_FORCEINLINE ImVec2 GetRange() const noexcept { return this->m_Range; }
        OV_FORCEINLINE bool IsPlaying() const noexcept { return this->m_Play; }
        void NeedUpdate(const bool need = true) noexcept;

        // Play/Pause functions
        OV_FORCEINLINE void Play() noexcept;
        OV_FORCEINLINE void Pause() noexcept;
        OV_FORCEINLINE void GoPreviousFrame() noexcept;
        OV_FORCEINLINE void GoNextFrame() noexcept;
        OV_FORCEINLINE void GoFirstFrame() noexcept;
        OV_FORCEINLINE void GoLastFrame() noexcept;
        OV_FORCEINLINE void SetFrame() noexcept;
        OV_FORCEINLINE uint32_t GetCurrentFrame() const noexcept;

        void BackgroundTimeUpdate() noexcept;
        
        void Release() noexcept;

    private:
        tsl::robin_map<uint32_t, Sequence*> m_Sequences;

        std::vector<uint8_t> m_CachedIndices;

		std::thread m_PlayerThread;

		std::mutex m_Mutex;

		std::condition_variable m_CV;

		Core::Loader* m_Loader = nullptr;

        ImVec2 m_Range;

        Logger* m_Logger = nullptr;

        float m_RealFramerate = 24.0f;

        uint32_t m_SequenceCount = 0;
        uint32_t m_Frame = 0;

        uint8_t m_FrameRate = 24;

        bool m_Play = false;
		bool m_Pause = false;
		bool m_Update = true;
		bool m_Release = false;
    };
}