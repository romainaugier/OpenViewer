// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewerApp/openviewerapp.h"

LOVA_NAMESPACE_BEGIN

class LOVA_API MediaPoolWidget 
{
public:
    MediaPoolWidget();

    ~MediaPoolWidget();

    void draw() noexcept;

    LOVA_FORCEINLINE void show(bool show = true) noexcept { this->m_show = show; }

private:
    bool m_show = true;
};

LOVA_NAMESPACE_END