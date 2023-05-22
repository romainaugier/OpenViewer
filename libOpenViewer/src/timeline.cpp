// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/timeline.h"
#include "OpenViewer/settings.h"

LOV_NAMESPACE_BEGIN

TimelineEventCallback::TimelineEventCallback(const TimelineEventCallbackFunc func,
                                             const TimelineEventType event)
{
    this->m_func_ptr = std::move(func);
    this->m_event_type = std::move(event);
}

Timeline::Timeline()
{
    this->m_play_thread = std::thread(Timeline::main_loop, this);
}

Timeline::Timeline(const uint8_t fps) : Timeline()
{ 
    this->set_fps(fps);
}

Timeline::~Timeline()
{
    {
        std::unique_lock<std::mutex> lock(this->m_lock);

        this->set_flag(TimelineFlag_Stop);
    }

    this->m_play_thread.join();
}

void Timeline::add_item(const TimelineItem item) noexcept
{
    this->m_items.push_back(std::move(item));
}

TimelineItem* Timeline::get_item_at_frame(const uint32_t frame) const noexcept
{
    for(auto item : this->m_items)
    {
        if(item.in_range(frame)) return &item;
    }

    return nullptr;
}

void Timeline::play() noexcept
{
    std::unique_lock<std::mutex> lock(this->m_lock);

    this->set_flag(TimelineFlag_Play);
}

void Timeline::pause() noexcept
{
    std::unique_lock<std::mutex> lock(this->m_lock);

    this->unset_flag(TimelineFlag_Play);
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

}

void Timeline::set_focus_range(const uint32_t start, const uint32_t end) noexcept
{

}

void Timeline::set_fps(const uint8_t fps) noexcept
{

}

void Timeline::set_frame(const uint32_t frame) noexcept
{
    std::unique_lock<std::mutex> lock(this->m_lock);
    
    this->m_frame = frame;

    TimelineItem* current_item = this->get_item_at_frame(frame);

    spdlog::debug("Executing FrameChanged callbacks");

    for (const auto &callback : this->m_event_callbacks)
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

void Timeline::main_loop()
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

            const auto end = std::chrono::steady_clock::now();
            const uint32_t elapsed = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::microseconds>(end  - start).count());

            const uint32_t loop_time = 1000 / this->m_fps;

            std::this_thread::sleep_for(std::chrono::milliseconds(loop_time - elapsed));
        }
    }
}

LOV_NAMESPACE_END
