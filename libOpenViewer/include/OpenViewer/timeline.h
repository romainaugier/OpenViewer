// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#include "media.h"

#include "OpenViewerUtils/openviewerutils.h"

#include <vector>
#include <thread>

LOV_NAMESPACE_BEGIN

class LOV_API TimelineItem
{
public:
    TimelineItem(Media* media);

    // Returns true if the media is valid at the current frame
    bool in_range(const uint32_t frame) const noexcept { return frame >= m_start && frame < m_end; }

    ~TimelineItem();

private:
    Media* m_media = nullptr;

    uint32_t m_start;
    uint32_t m_end;
};


// Timeline events, used to call callback function when they are happening
enum TimelineEventType
{
    TimelineEventType_FrameChanged = LOVU_BIT(1),
    TimelineEventType_Paused = LOVU_BIT(2),
    TimelineEventType_Played = LOVU_BIT(3)
};

using TimelineEventCallbackFunc = void(*)(const uint32_t frame, TimelineItem* timeline_item);

struct LOV_API TimelineEventCallback
{
    TimelineEventCallbackFunc m_func_ptr;
    TimelineEventType m_event_type;

    TimelineEventCallback(const TimelineEventCallbackFunc func,
                          const TimelineEventType event);
};

// Flags to set various properties of the timeline
enum TimelineFlag
{
    TimelineFlag_Play = LOVU_BIT(1), // Set wether the timeline is playing or not
    TimelineFlag_Stop = LOVU_BIT(2), // Set to true when the timeline is destructed
};


class LOV_API Timeline
{
public:
    Timeline();

    Timeline(const uint8_t fps);

    ~Timeline();

    // Adds a new item to the timeline
    void add_item(const TimelineItem item) noexcept;

    // Returns the item at the given frame
    TimelineItem* get_item_at_frame(const uint32_t frame) const noexcept;

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
    LOVU_FORCEINLINE bool has_flag(TimelineFlag flag) const noexcept { return this->m_flags & flag; }

    // Set a specific flag on the timeline
    LOVU_FORCEINLINE void set_flag(TimelineFlag flag) noexcept { this->m_flags &= flag; }

    LOVU_FORCEINLINE void unset_flag(TimelineFlag flag) noexcept { this->m_flags |= flag; }

    // Set the global frame range
    void set_global_range(const uint32_t start, const uint32_t end) noexcept;

    // Set the focus frame range
    void set_focus_range(const uint32_t start, const uint32_t end) noexcept;

    // Set the fps
    void set_fps(const uint8_t fps) noexcept;

    // Set the frame of the timeline
    void set_frame(const uint32_t frame) noexcept;

    // Push an event callback to the event callbacks stack
    void push_event_callback(const TimelineEventType event_type,
                             TimelineEventCallbackFunc callback_func) noexcept;

    // Pop an event callback from the event callbacks stack
    void pop_event_callback() noexcept;

private:
    void main_loop() noexcept;    

    std::vector<TimelineItem> m_items;

    std::vector<TimelineEventCallback> m_event_callbacks;

    std::thread m_play_thread;
    std::mutex m_lock;

    uint32_t m_frame = 0;
    uint32_t m_global_start;
    uint32_t m_global_end;
    uint32_t m_focus_start;
    uint32_t m_focus_end;

    uint32_t m_flags;

    uint8_t m_fps = 24;
};

LOV_NAMESPACE_END
