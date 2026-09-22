// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/log.hpp"

#include "stdromano/expected.hpp"
#include "stdromano/filesystem.hpp"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/dist_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <array>
#include <atomic>
#include <cctype>
#include <cstdlib>
#include <mutex>
#include <string>
#include <string_view>

LOV_NAMESPACE_BEGIN

LOG_NAMESPACE_BEGIN

static constexpr const char* PATTERN = "[%T.%e] [%^%l%$] [ov::%n] [%t] %v";

static constexpr std::size_t NUM_CATEGORIES = static_cast<std::size_t>(LogCategory::Count);

struct Registry
{
    std::shared_ptr<spdlog::sinks::dist_sink_mt> sink;
    std::shared_ptr<spdlog::sinks::basic_file_sink_mt> file_sink;
    std::array<std::shared_ptr<spdlog::logger>, NUM_CATEGORIES> loggers;
    stdromano::StringD file_path;
    std::mutex mutex;

    Registry()
    {
        this->sink = std::make_shared<spdlog::sinks::dist_sink_mt>();

        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_pattern(PATTERN);
        this->sink->add_sink(console_sink);

        for(std::size_t i = 0; i < NUM_CATEGORIES; ++i)
        {
            const char* name = category_name(static_cast<LogCategory>(i));

            this->loggers[i] = std::make_shared<spdlog::logger>(name, this->sink);
            this->loggers[i]->set_level(spdlog::level::info);
            this->loggers[i]->flush_on(spdlog::level::warn);
        }
    }
};

static Registry& registry() noexcept
{
    static Registry* instance = new Registry();

    return *instance;
}

static std::atomic<bool> g_initialized{false};

spdlog::logger& get(LogCategory category) noexcept
{
    const std::size_t index = static_cast<std::size_t>(category);

    Registry& reg = registry();

    return *reg.loggers[index < NUM_CATEGORIES ? index
                                               : static_cast<std::size_t>(LogCategory::App)];
}

void set_level(spdlog::level::level_enum level) noexcept
{
    for(auto& logger : registry().loggers)
        logger->set_level(level);
}

void set_level(LogCategory category, spdlog::level::level_enum level) noexcept
{
    get(category).set_level(level);
}

static std::string_view trim(std::string_view s) noexcept
{
    while(!s.empty() && std::isspace(static_cast<unsigned char>(s.front())))
        s.remove_prefix(1);

    while(!s.empty() && std::isspace(static_cast<unsigned char>(s.back())))
        s.remove_suffix(1);

    return s;
}

static bool iequals(std::string_view lhs, std::string_view rhs) noexcept
{
    if(lhs.size() != rhs.size())
        return false;

    for(std::size_t i = 0; i < lhs.size(); ++i)
        if(std::tolower(static_cast<unsigned char>(lhs[i])) !=
           std::tolower(static_cast<unsigned char>(rhs[i])))
            return false;

    return true;
}

static bool parse_level(std::string_view name, spdlog::level::level_enum& out) noexcept
{
    static constexpr std::pair<const char*, spdlog::level::level_enum> LEVELS[] = {
        {"trace", spdlog::level::trace},
        {"debug", spdlog::level::debug},
        {"info", spdlog::level::info},
        {"warn", spdlog::level::warn},
        {"warning", spdlog::level::warn},
        {"error", spdlog::level::err},
        {"err", spdlog::level::err},
        {"critical", spdlog::level::critical},
        {"off", spdlog::level::off},
    };

    for(const auto& level : LEVELS)
    {
        if(iequals(name, level.first))
        {
            out = level.second;
            return true;
        }
    }

    return false;
}

static bool parse_category(std::string_view name, std::size_t& out) noexcept
{
    for(std::size_t i = 0; i < NUM_CATEGORIES; ++i)
    {
        if(iequals(name, category_name(static_cast<LogCategory>(i))))
        {
            out = i;
            return true;
        }
    }

    return false;
}

bool apply_levels(const char* spec) noexcept
{
    if(spec == nullptr)
        return false;

    std::array<spdlog::level::level_enum, NUM_CATEGORIES> levels;
    std::array<bool, NUM_CATEGORIES> changed{};

    for(std::size_t i = 0; i < NUM_CATEGORIES; ++i)
        levels[i] = registry().loggers[i]->level();

    std::string_view remaining(spec);

    while(!remaining.empty())
    {
        const std::size_t comma = remaining.find(',');
        const std::string_view entry = trim(remaining.substr(0, comma));
        remaining =
            comma == std::string_view::npos ? std::string_view() : remaining.substr(comma + 1);

        if(entry.empty())
            continue;

        const std::size_t equal = entry.find('=');

        spdlog::level::level_enum level;

        if(equal == std::string_view::npos)
        {
            if(!parse_level(entry, level))
                return false;

            levels.fill(level);
            changed.fill(true);
            continue;
        }

        std::size_t category;

        if(!parse_category(trim(entry.substr(0, equal)), category) ||
           !parse_level(trim(entry.substr(equal + 1)), level))
            return false;

        levels[category] = level;
        changed[category] = true;
    }

    for(std::size_t i = 0; i < NUM_CATEGORIES; ++i)
        if(changed[i])
            registry().loggers[i]->set_level(levels[i]);

    return true;
}

void initialize(spdlog::level::level_enum level) noexcept
{
    Registry& reg = registry();

    set_level(level);

    bool expected = false;

    if(g_initialized.compare_exchange_strong(expected, true))
    {
        try
        {
            const stdromano::Expected<stdromano::StringD> tmp_dir = stdromano::fs::tmp_dir();

            if(tmp_dir.has_value())
            {
                std::lock_guard<std::mutex> lock(reg.mutex);

                reg.file_path = tmp_dir.value().copy();
                reg.file_path.appendc("/openviewer.log");

                reg.file_sink =
                    std::make_shared<spdlog::sinks::basic_file_sink_mt>(reg.file_path.c_str(),
                                                                        true);
                reg.file_sink->set_pattern(PATTERN);

                reg.sink->add_sink(reg.file_sink);
            }
            else
            {
                std::fprintf(stderr, "Cannot find a temporary directory for the log file\n");
            }
        }
        catch(const std::exception& e)
        {
            std::fprintf(stderr, "Could not create the log file: %s\n", e.what());
        }

        static std::once_flag atexit_flag;
        std::call_once(atexit_flag, []() -> void { std::atexit([]() -> void { shutdown(); }); });
    }

    const char* env_levels = std::getenv("OPENVIEWER_LOG");

    if(env_levels != nullptr && !apply_levels(env_levels))
    {
        std::string valid;

        for(std::size_t i = 0; i < NUM_CATEGORIES; ++i)
            valid.append(i > 0 ? ", " : "").append(category_name(static_cast<LogCategory>(i)));

        std::fprintf(stderr,
                     "Ignoring OPENVIEWER_LOG=\"%s\": expected \"<level>\" or "
                     "\"<category>=<level>,...\" with categories %s and levels "
                     "trace, debug, info, warn, error, critical, off\n",
                     env_levels,
                     valid.c_str());
    }
}

void flush() noexcept
{
    for(auto& logger : registry().loggers)
        logger->flush();
}

void shutdown() noexcept
{
    if(!g_initialized.exchange(false))
        return;

    Registry& reg = registry();

    flush();

    std::lock_guard<std::mutex> lock(reg.mutex);

    if(reg.file_sink != nullptr)
    {
        reg.sink->remove_sink(reg.file_sink);
        reg.file_sink.reset();
    }
}

const stdromano::StringD& file_path() noexcept
{
    return registry().file_path;
}

LOG_NAMESPACE_END

LOV_NAMESPACE_END
