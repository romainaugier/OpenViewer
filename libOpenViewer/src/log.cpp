// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/log.hpp"

#include "stdromano/filesystem.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

LOV_NAMESPACE_BEGIN

LOG_NAMESPACE_BEGIN

// TODO: use async logger in release for better performances

void initialize(spdlog::level::level_enum level) noexcept
{
    spdlog::set_pattern("[%T] [%^%l%$] [ov] %v");

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_pattern("[%T] [%^%l%$] [ov::%n] %v");

    auto tmp_file_path = stdromano::fs_tmp_dir().copy();
    tmp_file_path.appendc("/openviewer.log");

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(tmp_file_path.c_str());

    auto media_log = std::make_shared<spdlog::logger>("media", spdlog::sinks_init_list{ console_sink, file_sink });
    spdlog::register_logger(media_log);

    auto media_cache_log = std::make_shared<spdlog::logger>("media_cache", spdlog::sinks_init_list{ console_sink, file_sink });
    spdlog::register_logger(media_cache_log);

    auto media_pool_log = std::make_shared<spdlog::logger>("media_pool", spdlog::sinks_init_list{ console_sink, file_sink });
    spdlog::register_logger(media_pool_log);

    spdlog::set_level(level);
}

LOG_NAMESPACE_END

LOV_NAMESPACE_END
