// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "app.h"

namespace Interface
{
    Application::Application(Logger* logger)
    {
        this->m_Logger = logger;

        this->m_Windows["Image Infos"] = false;
        this->m_Windows["Sequence Infos"] = false;
    }
}