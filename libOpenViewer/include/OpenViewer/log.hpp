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

// Calls below this level are compiled out: trace in release builds by default.
// Override with -DLOV_LOG_ACTIVE_LEVEL=SPDLOG_LEVEL_<LEVEL>. The arguments of a
// compiled out call are still evaluated, so keep expensive ones out of trace calls.
#if !defined(LOV_LOG_ACTIVE_LEVEL)
#if defined(NDEBUG)
#define LOV_LOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#else
#define LOV_LOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#endif // defined(NDEBUG)
#endif // !defined(LOV_LOG_ACTIVE_LEVEL)

LOV_NAMESPACE_BEGIN

// One logger per part of the library, printed as [ov::<name>]. An enum rather than
// strings: a typo in a category is a compile error, not a new silent logger.
enum class LogCategory : std::uint8_t
{
    Media,
    MediaCache,
    MediaPool,
    ImageReader,
    App,
    Count,
};

LOG_NAMESPACE_BEGIN

LOV_FORCE_INLINE constexpr const char* category_name(LogCategory category) noexcept
{
    switch(category)
    {
        case LogCategory::Media:
            return "media";
        case LogCategory::MediaCache:
            return "media_cache";
        case LogCategory::MediaPool:
            return "media_pool";
        case LogCategory::ImageReader:
            return "image_reader";
        case LogCategory::App:
            return "app";
        default:
            return "unknown";
    }
}

// Usable before initialize(): every logger exists from the first call and writes to
// the console. initialize() adds the file sink.
LOV_API spdlog::logger& get(LogCategory category) noexcept;

// Adds the file sink, sets every category to level, then applies the OPENVIEWER_LOG
// environment variable on top (see apply_levels). Idempotent.
LOV_API void initialize(spdlog::level::level_enum level = spdlog::level::info) noexcept;

// Flushes and removes the file sink. Registered with atexit() by initialize().
LOV_API void shutdown() noexcept;

LOV_API void set_level(spdlog::level::level_enum level) noexcept;

LOV_API void set_level(LogCategory category, spdlog::level::level_enum level) noexcept;

// Applies "debug" (every category) or "media_cache=trace,image_reader=debug".
// Returns false and changes nothing if any category or level name is unknown.
LOV_API bool apply_levels(const char* spec) noexcept;

LOV_API void flush() noexcept;

// Path the file sink writes to. Empty until initialize() has been called.
LOV_API const stdromano::StringD& file_path() noexcept;

LOG_NAMESPACE_END

#define LOV_LOG_FUNCTION(func_name, method, spdlog_level)                                          \
    template <typename... Args>                                                                    \
    LOV_FORCE_INLINE void func_name(LogCategory category,                                          \
                                    spdlog::format_string_t<Args...> fmt,                          \
                                    Args&&... args) noexcept                                       \
    {                                                                                              \
        if constexpr (LOV_LOG_ACTIVE_LEVEL <= spdlog_level)                                        \
        {                                                                                          \
            log::get(category).method(fmt, std::forward<Args>(args)...);                           \
        }                                                                                          \
        else                                                                                       \
        {                                                                                          \
            LOV_UNUSED(category);                                                                  \
            LOV_UNUSED(fmt);                                                                       \
            (LOV_UNUSED(args), ...);                                                               \
        }                                                                                          \
    }

LOV_LOG_FUNCTION(log_error, error, SPDLOG_LEVEL_ERROR)
LOV_LOG_FUNCTION(log_warn, warn, SPDLOG_LEVEL_WARN)
LOV_LOG_FUNCTION(log_info, info, SPDLOG_LEVEL_INFO)
LOV_LOG_FUNCTION(log_debug, debug, SPDLOG_LEVEL_DEBUG)
LOV_LOG_FUNCTION(log_trace, trace, SPDLOG_LEVEL_TRACE)

#undef LOV_LOG_FUNCTION

LOV_NAMESPACE_END

#endif // !defined(__LOV_LOG)
