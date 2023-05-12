// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/timeline.h"
#include "OpenViewer/settings.h"

LOV_NAMESPACE_BEGIN

Timeline::Timeline()
{

}

Timeline::Timeline(const uint8_t fps)
{
    this->m_fps = fps;
}

void Timeline::add_item(const TimelineItem& item) noexcept
{
    this->m_items.push_back(std::move(item));
}

void Timeline::play() noexcept
{

}

void Timeline::pause() noexcept
{

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

LOV_NAMESPACE_END
