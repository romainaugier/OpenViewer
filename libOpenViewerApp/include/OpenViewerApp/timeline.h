// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewerApp/openviewerapp.h"
#include "OpenViewer/timeline.h"

LOVA_NAMESPACE_BEGIN

class LOVA_API TimelineWidget
{
public:
    lov::Timeline internal_timeline; 

    TimelineWidget();

    ~TimelineWidget();

    void draw() noexcept;

    LOVA_FORCEINLINE void show(bool show = true) noexcept { this->m_show = show; }

private:
    float m_pan = 0.0f;
    float m_zoom = 1.0;

    bool m_show = true;
};

LOVA_NAMESPACE_END