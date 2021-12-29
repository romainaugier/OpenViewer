// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "media.h"

namespace Core
{
    uint32_t Media::Size() const noexcept
    {
        return this->m_Range.y;
    }

    bool Media::InRange(const uint32_t index) const noexcept
    {
        return index >= this->m_TimelineRange.x && index < this->m_TimelineRange.y;
    }

    void Media::SetActive() noexcept
    {
        this->m_IsActive = true;
        this->m_TimelineRange = this->m_Range;
    }
    
    void Media::SetInactive() noexcept
    {
        this->m_IsActive = false;
        this->m_TimelineRange = ImVec2(0, 0);
    }
}