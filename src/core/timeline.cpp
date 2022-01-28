// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 Romain Augier
// All rights reserved.

#include "timeline.h"

namespace Core
{
    void Timeline::Initialize(const ImVec2& range) noexcept
    {
        this->m_Range = range;
    }

    void Timeline::Add(Media* media) noexcept
    {
        Sequence* newSequence = new Sequence;
        newSequence->m_Media = media;
        newSequence->m_Range = media->GetRange();

        this->m_Sequences[media->Path()] = newSequence; 
    }

    void Timeline::Remove(Media* media) noexcept
    {

    }
}