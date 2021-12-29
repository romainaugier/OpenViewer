// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include <unordered_map>

#include "GLFW/glfw3.h"

namespace Interface
{
    using ShortcutCallback = void(*)(void*);
    using Map = std::unordered_map<std::string, ShortcutCallback>;

    struct Shortcuts
    {
        Map m_ShortcutMap;

        void Register() noexcept;

        void Handle() noexcept;
    };
}