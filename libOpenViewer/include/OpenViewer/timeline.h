// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "media.h"

LOV_NAMESPACE_BEGIN

class LOV_DLL TimelineItem
{
public:
    TimelineItem(Media* media);

    ~TimelineItem();



private:
    Media* m_media = nullptr;

    uint32_t m_start;
    uint32_t m_end;
};

class LOV_DLL Timeline
{

};

LOV_NAMESPACE_END