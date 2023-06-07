// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/display.h"

LOV_NAMESPACE_BEGIN

void Display::set_data(void* data_ptr,
                       const uint32_t data_width,
                       const uint32_t data_height,
                       const DisplayDataType data_type) noexcept
{
    const bool need_update = this->m_width != data_width ? true : 
                             this->m_height != data_height ? true :
                             this->m_data_type != data_type ? true : false;

    this->m_data = data_ptr;
    this->m_width = data_width;
    this->m_height = data_height;
    this->m_data_type = data_type;

    if(need_update)
    {
        
    }
}

LOV_NAMESPACE_END