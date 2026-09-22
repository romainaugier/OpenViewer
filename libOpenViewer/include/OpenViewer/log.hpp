// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__LOV_LOG)
#define __LOV_LOG

#include "OpenViewer/common.hpp"

#include "stdromano/string.hpp"

// spdlog is vendored by stdromano (in ext/spdlog) and its headers are copied into
// the stdromano install include dir, so it comes for free with stdromano::stdromano

#include <spdlog/spdlog.h>

#define LOG_NAMESPACE_BEGIN namespace log {
#define LOG_NAMESPACE_END }

LOV_NAMESPACE_BEGIN

LOG_NAMESPACE_BEGIN

// Installs the console + file sinks on the default logger. Idempotent, so every
// test binary can call it unconditionally without worrying about ordering.
LOV_API void initialize(spdlog::level::level_enum level = spdlog::level::info) noexcept;

// Flushes and drops every registered logger. Registered with atexit() by
// initialize(); exposed for tests that want a deterministic teardown.
LOV_API void shutdown() noexcept;

// Path the file sink writes to. Empty until initialize() has been called.
LOV_API const stdromano::StringD& file_path() noexcept;

LOG_NAMESPACE_END

template <typename... Args>
LOV_FORCE_INLINE void log_error(spdlog::format_string_t<Args...> fmt, Args&&... args) noexcept
{
    spdlog::error(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
LOV_FORCE_INLINE void log_warn(spdlog::format_string_t<Args...> fmt, Args&&... args) noexcept
{
    spdlog::warn(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
LOV_FORCE_INLINE void log_info(spdlog::format_string_t<Args...> fmt, Args&&... args) noexcept
{
    spdlog::info(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
LOV_FORCE_INLINE void log_debug(spdlog::format_string_t<Args...> fmt, Args&&... args) noexcept
{
    spdlog::debug(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
LOV_FORCE_INLINE void log_trace(spdlog::format_string_t<Args...> fmt, Args&&... args) noexcept
{
    spdlog::trace(fmt, std::forward<Args>(args)...);
}

LOV_NAMESPACE_END

#endif // !defined(__LOV_LOG)
