// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "shortcuts.h"

namespace Interface
{
    void Shortcuts::Keys(GLFWwindow* window, int key, int scancode, int action, int mods) noexcept
    {
        if (key == GLFW_KEY_UNKNOWN) return;

        if (action == GLFW_PRESS) m_Pressed[key] = true;
        else if (action == GLFW_RELEASE) m_Pressed[key] = false;
    }
    
    void Shortcuts::Handle() noexcept
    {
        
    }
}