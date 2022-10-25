// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Romain Augier
// All rights reserved.

#pragma once

#include <chrono>
#include "tsl/robin_map.h"

#include "openviewerutils.h"

LOVU_NAMESPACE_BEGIN

// Very simple profiler to get timings of some actions at runtime to help finding hotspots in the application
struct LOVU_DLL Profiler
{
    tsl::robin_map<std::string, float> times;
    tsl::robin_map<std::string, float> mem_usage;

    // Adds a memory usage to the profiler
    LOVU_FORCEINLINE void add_mem_usage(const std::string& name, float memory) noexcept;

    // Starts a timer
    LOVU_FORCEINLINE auto start() const noexcept;

    // Ends a timer
    LOVU_FORCEINLINE auto end() const noexcept;

    // Adds a time to the profiler
    LOVU_FORCEINLINE void time(const std::string& name,
                                const std::chrono::time_point<std::chrono::system_clock>& start, 
                                const std::chrono::time_point<std::chrono::system_clock>& end) noexcept;
};

LOVU_NAMESPACE_END