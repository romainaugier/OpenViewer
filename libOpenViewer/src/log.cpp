// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/log.hpp"

#include "stdromano/expected.hpp"
#include "stdromano/filesystem.hpp"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <atomic>

LOV_NAMESPACE_BEGIN

LOG_NAMESPACE_BEGIN

static std::atomic<bool> g_initialized{false};
static stdromano::StringD g_file_path;

void initialize(spdlog::level::level_enum level) noexcept
{
    bool expected = false;

    if(!g_initialized.compare_exchange_strong(expected, true))
    {
        // Already initialized; only honour the new level.
        spdlog::set_level(level);
        return;
    }

    try
    {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

        const stdromano::Expected<stdromano::StringD> tmp_dir = stdromano::fs::tmp_dir();

        if(!tmp_dir.has_value())
        {
            std::fprintf(stderr, "Cannot find a temporary directory for the log file\n");
            spdlog::set_level(level);
            return;
        }

        g_file_path = tmp_dir.value().copy();
        g_file_path.appendc("/openviewer.log");

        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(g_file_path.c_str(),
                                                                             true);

        auto logger = std::make_shared<spdlog::logger>("ov", spdlog::sinks_init_list{ console_sink, file_sink });

        logger->set_pattern("[%T.%e] [%^%l%$] [ov] [%t] %v");
        logger->set_level(level);
        logger->flush_on(spdlog::level::warn);

        spdlog::set_default_logger(logger);
        spdlog::set_level(level);
    }
    catch(const std::exception& e)
    {
        // Losing the file sink must never take the process down: fall back to
        // whatever spdlog gives us by default and carry on.
        std::fprintf(stderr, "Could not initialize the logger: %s\n", e.what());
        spdlog::set_level(level);
    }

    std::atexit([]() -> void { shutdown(); });
}

void shutdown() noexcept
{
    if(!g_initialized.exchange(false))
        return;

    spdlog::shutdown();
}

const stdromano::StringD& file_path() noexcept
{
    return g_file_path;
}

LOG_NAMESPACE_END

LOV_NAMESPACE_END
