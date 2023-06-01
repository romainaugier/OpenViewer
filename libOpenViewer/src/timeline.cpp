// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/timeline.h"
#include "OpenViewer/settings.h"

LOV_NAMESPACE_BEGIN

TimelineItem::TimelineItem(Media* media)
{
    this->m_media = media;
    this->m_start = media->get_start_frame();
    this->m_end = media->get_end_frame();
}

TimelineItem::~TimelineItem()
{

}

TimelineEventCallback::TimelineEventCallback(const TimelineEventCallbackFunc func,
                                             const TimelineEventType event)
{
    this->m_func_ptr = func;
    this->m_event_type = event;
}

Timeline::Timeline()
{
    this->m_play_thread = std::thread(&Timeline::main_loop, this);
}

Timeline::Timeline(const uint8_t fps) : Timeline()
{ 
    this->set_fps(fps);
}

Timeline::~Timeline()
{
    this->set_flag(TimelineFlag_Stop);

    this->m_play_thread.join();
}

void Timeline::add_item(const TimelineItem item) noexcept
{
    spdlog::debug("Added a new item to the timeline : {}", item.get_media()->make_path_at_frame(item.get_start_frame()));    

    this->m_items.push_back(std::move(item));
}

TimelineItem* Timeline::get_item_at_frame(const uint32_t frame) noexcept
{
    for(auto it = this->m_items.begin(); it != this->m_items.end(); ++it)
    {
        if(it->in_range(frame))
        {
            return &(*it);
        }
    }

    return nullptr;
}

void Timeline::play() noexcept
{
    spdlog::debug("[TIMELINE] : Playing");

    this->set_flag(TimelineFlag_Play);
}

void Timeline::pause() noexcept
{
    this->unset_flag(TimelineFlag_Play);

    spdlog::debug("[TIMELINE] : Pausing");
}

void Timeline::go_to_frame(const uint32_t frame) noexcept
{

}

void Timeline::go_to_first_frame() noexcept
{

}

void Timeline::go_to_last_frame() noexcept
{

}

void Timeline::set_global_range(const uint32_t start, const uint32_t end) noexcept
{
    std::unique_lock<std::mutex> lock(this->m_lock);

    this->m_global_start = start;
    this->m_global_end = end;
}

void Timeline::set_focus_range(const uint32_t start, const uint32_t end) noexcept
{
    std::unique_lock<std::mutex> lock(this->m_lock);

    this->m_focus_start = start;
    this->m_focus_end = end;
}

void Timeline::fit_ranges_to_items() noexcept
{
    uint32_t start_frame = 0xffffffff;
    uint32_t end_frame = 0;

    for(const auto& item : this->m_items)
    {
        if(item.get_start_frame() < start_frame) start_frame = item.get_start_frame();
        if(item.get_end_frame() > end_frame) end_frame = item.get_end_frame();
    }

    this->set_focus_range(start_frame, end_frame);
    this->set_global_range(start_frame, end_frame);
}

void Timeline::set_fps(const uint8_t fps) noexcept
{
    std::unique_lock<std::mutex> lock(this->m_lock);

    this->m_fps = fps;
}

void Timeline::set_frame(const uint32_t frame) noexcept
{
    std::unique_lock<std::mutex> lock(this->m_lock);
    
    this->m_frame = frame;

    TimelineItem* current_item = this->get_item_at_frame(frame);

    spdlog::debug("Executing FrameChanged callbacks");

    for (const auto& callback : this->m_event_callbacks)
    {
        if (callback.m_event_type & TimelineEventType_FrameChanged)
        {
            callback.m_func_ptr(this->m_frame, current_item);
        }
    }
}

void Timeline::push_event_callback(const TimelineEventType event_type,
                                   TimelineEventCallbackFunc callback_func) noexcept
{
    this->m_event_callbacks.emplace_back(callback_func, event_type);
}

void Timeline::pop_event_callback() noexcept
{
    this->m_event_callbacks.erase(this->m_event_callbacks.end());
}

void Timeline::main_loop() noexcept
{
    spdlog::debug("Starting Timeline main thread");

    while(true)
    {
        const auto start = std::chrono::steady_clock::now();

        if(this->has_flag(TimelineFlag_Stop))
        {
            spdlog::debug("Releasing Timeline main thread");
            break;
        }

        if(this->has_flag(TimelineFlag_Play))
        {
            this->set_frame(this->m_frame + 1);
        }

        spdlog::debug("Flags : {}", this->m_flags);

        const auto end = std::chrono::steady_clock::now();
        const uint32_t elapsed = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::microseconds>(end  - start).count());

        const uint32_t loop_time = (uint32_t)1000 / (uint32_t)this->m_fps;
        const uint32_t sleep_time = elapsed > loop_time ? 0 : loop_time - elapsed;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
    }
}

LOV_NAMESPACE_END
