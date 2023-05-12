// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#include "media.h"

#include "OpenViewerUtils/openviewerutils.h"

#include <vector>
#include <thread>
#include <stack>

LOV_NAMESPACE_BEGIN

class LOV_API TimelineItem
{
public:
    TimelineItem(Media* media);

    ~TimelineItem();

private:
    Media* m_media = nullptr;

    uint32_t m_start;
    uint32_t m_end;
};

enum TimelineFlag
{
    TimelineFlag_PlayOnce = LOVU_BIT(1),
    TimelineFlag_PlayLoop = LOVU_BIT(2),
    TimelineFlag_PlayPingPong = LOVU_BIT(3)
};

using timeline_on_play_callback_func = void(*)(void*, const uint32_t frame, TimelineItem& timeline_item);

class LOV_API Timeline
{
public:
    Timeline();

    Timeline(const uint8_t fps);

    ~Timeline();

    // Adds a new item to the timeline
    void add_item(const TimelineItem& item) noexcept;

    // Plays the timeline
    void play() noexcept;

    // Pauses the timeline
    void pause() noexcept;

    // Go to a specific frame on the timeline
    void go_to_frame(const uint32_t frame) noexcept;

    // Go to the first frame on the timeline
    void go_to_first_frame() noexcept;

    // Go to the last frame on the timeline
    void go_to_last_frame() noexcept;

    // Check if the timeline has a specific flag set
    LOVU_FORCEINLINE bool has_flag(TimelineFlag flag) { return this->m_flags & flag; }

    // Set a specific flag on the timeline
    LOVU_FORCEINLINE void set_flag(TimelineFlag flag) { this->m_flags &= flag; }

    // Set the global frame range
    void set_global_range(const uint32_t start, const uint32_t end) noexcept;

    // Set the focus frame range
    void set_focus_range(const uint32_t start, const uint32_t end) noexcept;

    // Set the fps
    void set_fps(const uint8_t fps) noexcept;

private:
    std::vector<TimelineItem> m_items;

    std::thread m_play_thread;
    std::mutex m_lock;

    uint32_t m_global_start;
    uint32_t m_global_end;
    uint32_t m_focus_start;
    uint32_t m_focus_end;

    uint32_t m_flags;

    uint8_t m_fps = 24;
};

LOV_NAMESPACE_END
