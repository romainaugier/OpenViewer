// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Romain Augier
// All rights reserved.

#include "OpenViewerUtils/profiler.h"

LOVU_NAMESPACE_BEGIN

LOVU_FORCEINLINE void Profiler::add_mem_usage(const std::string& name, float memory) noexcept
{
    this->mem_usage[name] = memory;
}

LOVU_FORCEINLINE auto Profiler::start() const noexcept
{
    return std::chrono::system_clock::now();
}

LOVU_FORCEINLINE auto Profiler::end() const noexcept
{
    return std::chrono::system_clock::now();
}

LOVU_FORCEINLINE void Profiler::time(const std::string& name,
                                    const std::chrono::time_point<std::chrono::system_clock>& start, 
                                    const std::chrono::time_point<std::chrono::system_clock>& end) noexcept
{
    const float time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    this->times[name] = time;
}

LOVU_NAMESPACE_END