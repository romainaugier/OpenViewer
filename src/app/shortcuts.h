// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include "tsl/robin_map.h"

#include "GLFW/glfw3.h"

#define GLFW_KEY_COUNT 349

namespace Interface
{
    using ShortcutCallback = void(*)(void*);
    using Map = tsl::robin_map<std::string, ShortcutCallback>;

    struct Shortcuts
    {
        Map m_ShortcutMap;

        bool m_Pressed[GLFW_KEY_COUNT];

        void Register() noexcept;

        void Keys(GLFWwindow* window, int key, int scancode, int action, int mods) noexcept;

        void Handle() noexcept;
    };
}