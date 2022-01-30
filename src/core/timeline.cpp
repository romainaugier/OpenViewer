// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 Romain Augier
// All rights reserved.

#include "timeline.h"

namespace Core
{
    void Timeline::Initialize(const ImVec2& range, Logger* logger) noexcept
    {
        this->m_Logger = logger;
        this->m_Range = range;
    }

    void Timeline::Add(Media* media) noexcept
    {
        Sequence* newSequence = new Sequence;
        newSequence->m_Media = media;
        newSequence->m_Range = media->GetRange();

        this->m_Sequences[++this->m_SequenceCount] = newSequence; 
    }

    void Timeline::Remove(Media* media) noexcept
    {

    }

    void Timeline::Release() noexcept
    {
        for (auto it = this->m_Sequences.cbegin(); it != this->m_Sequences.cend(); ++it)
        {
            delete it.value();
        }

        this->m_Logger->Log(LogLevel_Diagnostic, "[TIMELINE] : Release timeline"),
    }
}