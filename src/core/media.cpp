// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "media.h"

namespace Core
{
    Media::Size() noexcept
    {
        return this->m_Range.y + 1; // The range starts at 0
    }

    Media::InRange(const uint32_t index) noexcept
    {
        return index >= this->m_TimelineRange.x && index <= this->m_TimelineRange.y;
    }
}