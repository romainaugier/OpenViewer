// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Romain Augier
// All rights reserved.

#pragma once

#include <chrono>

#include "openviewerutils.h"

LOVU_NAMESPACE_BEGIN

struct ScopedTimer
{
    ScopedTimer(std::string title)
    {
        this->title = std::move(title);
        this->start = std::chrono::steady_clock::now();
    }

    ~ScopedTimer()
    {
        const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - this->start).count();
        spdlog::debug("Scoped Timer \"{}\" : {} ms", this->title, elapsed / 1000.0f);
    }

    std::string title;
    std::chrono::time_point<std::chrono::steady_clock> start;
};

#define SCOPED_TIMER(title) lovu::ScopedTimer timer(title)
#define SCOPED_TIMER_STRINGIFY(title) lovu::ScopedTimer timer(#title)

template<typename F, typename ...Args>
static auto _func_timer(const char* func_name, 
                        F&& func, 
                        Args&&... args)
{
    SCOPED_TIMER(func_name);
    return func(std::forward<Args>(args)...);
}

#define func_timer(func, ...) _func_timer(#func, func, ##__VA_ARGS__)

LOVU_NAMESPACE_END