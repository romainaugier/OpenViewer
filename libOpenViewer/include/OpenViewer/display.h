// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/openviewer.h"

LOV_NAMESPACE_BEGIN

class Display
{

private:
    void* m_data;

    uint32_t m_width;
    uint32_t m_height;

    uint8_t m_bit_depth;
};

LOV_NAMESPACE_END