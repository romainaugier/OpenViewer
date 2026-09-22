// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "lov_test.hpp"

#include "OpenViewer/log.hpp"

#include <cstdlib>
#include <set>

using namespace lov;

static constexpr std::size_t NUM_CATEGORIES = static_cast<std::size_t>(LogCategory::Count);

static void set_env(const char* name, const char* value)
{
#if defined(LOV_WIN)
    _putenv_s(name, value);
#else
    setenv(name, value, 1);
#endif // defined(LOV_WIN)
}

static void unset_env(const char* name)
{
#if defined(LOV_WIN)
    _putenv_s(name, "");
#else
    unsetenv(name);
#endif // defined(LOV_WIN)
}

// Every test starts from the same levels, whatever the previous one left
static void reset_levels()
{
    log::set_level(spdlog::level::info);
}

static std::size_t count(const std::string& haystack, const std::string& needle)
{
    std::size_t n = 0;

    for(std::size_t pos = haystack.find(needle); pos != std::string::npos;
        pos = haystack.find(needle, pos + needle.size()))
        ++n;

    return n;
}

LOV_TEST(category_names_are_explicit)
{
    const char* expected[] = {"media", "media_cache", "media_pool", "image_reader", "app"};

    LOV_REQUIRE(sizeof(expected) / sizeof(expected[0]) == NUM_CATEGORIES);

    std::set<std::string> seen;

    for(std::size_t i = 0; i < NUM_CATEGORIES; ++i)
    {
        const LogCategory category = static_cast<LogCategory>(i);

        LOV_CHECK_EQ(std::string(log::category_name(category)), std::string(expected[i]));
        LOV_CHECK_EQ(log::get(category).name(), std::string(expected[i]));

        seen.insert(log::category_name(category));
    }

    LOV_CHECK_EQ(seen.size(), NUM_CATEGORIES);
}

LOV_TEST(every_category_is_named_in_the_file)
{
    reset_levels();

    for(std::size_t i = 0; i < NUM_CATEGORIES; ++i)
        log_info(static_cast<LogCategory>(i), "category_marker_{}", i);

    const std::string log = lov_test::read_log_file();

    for(std::size_t i = 0; i < NUM_CATEGORIES; ++i)
    {
        const std::string prefix = std::string("[ov::") + log::category_name(static_cast<LogCategory>(i)) + "]";

        LOV_CHECK(lov_test::log_has_line(log, prefix, "category_marker_" + std::to_string(i)));
    }
}

LOV_TEST(levels_are_per_category)
{
    reset_levels();

    log::set_level(LogCategory::MediaCache, spdlog::level::off);

    log_warn(LogCategory::MediaCache, "silenced_cache_marker");
    log_warn(LogCategory::Media, "audible_media_marker");

    const std::string log = lov_test::read_log_file();

    LOV_CHECK(log.find("silenced_cache_marker") == std::string::npos);
    LOV_CHECK(lov_test::log_has_line(log, "[ov::media]", "audible_media_marker"));

    reset_levels();
}

LOV_TEST(apply_levels_accepts_valid_specs)
{
    reset_levels();

    LOV_CHECK(log::apply_levels("media_cache=trace,image_reader=debug"));
    LOV_CHECK_EQ(log::get(LogCategory::MediaCache).level(), spdlog::level::trace);
    LOV_CHECK_EQ(log::get(LogCategory::ImageReader).level(), spdlog::level::debug);
    LOV_CHECK_EQ(log::get(LogCategory::Media).level(), spdlog::level::info);

    // A bare level applies to every category, later entries override it
    LOV_CHECK(log::apply_levels("warn, media = Debug"));
    LOV_CHECK_EQ(log::get(LogCategory::MediaCache).level(), spdlog::level::warn);
    LOV_CHECK_EQ(log::get(LogCategory::Media).level(), spdlog::level::debug);

    LOV_CHECK(log::apply_levels(""));

    reset_levels();
}

// A typo must not silently change anything: spdlog's own from_str maps unknown
// names to "off", which would disable logging for that category
LOV_TEST(apply_levels_rejects_typos_atomically)
{
    reset_levels();

    LOV_CHECK(!log::apply_levels("mediacache=trace"));
    LOV_CHECK(!log::apply_levels("media_cache=verbose"));
    LOV_CHECK(!log::apply_levels("loud"));
    LOV_CHECK(!log::apply_levels(nullptr));

    // The valid first entry must not be applied when the second one is wrong
    LOV_CHECK(!log::apply_levels("media=trace,media_cache=verbose"));

    for(std::size_t i = 0; i < NUM_CATEGORIES; ++i)
        LOV_CHECK_EQ(log::get(static_cast<LogCategory>(i)).level(), spdlog::level::info);
}

LOV_TEST(environment_variable_is_applied)
{
    set_env("OPENVIEWER_LOG", "media_cache=trace");
    log::initialize(spdlog::level::warn);

    LOV_CHECK_EQ(log::get(LogCategory::MediaCache).level(), spdlog::level::trace);
    LOV_CHECK_EQ(log::get(LogCategory::Media).level(), spdlog::level::warn);

    // Invalid: ignored with a message, the level passed to initialize() stands
    set_env("OPENVIEWER_LOG", "media_cache=nope");
    log::initialize(spdlog::level::warn);

    LOV_CHECK_EQ(log::get(LogCategory::MediaCache).level(), spdlog::level::warn);

    unset_env("OPENVIEWER_LOG");
    reset_levels();
}

// Loggers work before initialize() and after shutdown() (console only), and
// initializing again adds the file sink once, not twice
LOV_TEST(reinitializing_does_not_duplicate_the_file)
{
    log::shutdown();

    log_warn(LogCategory::Media, "logged_while_shut_down");

    log::initialize(spdlog::level::info);

    log_warn(LogCategory::Media, "logged_once_marker");

    const std::string log = lov_test::read_log_file();

    LOV_CHECK_EQ(count(log, "logged_once_marker"), std::size_t(1));
    LOV_CHECK(log.find("logged_while_shut_down") == std::string::npos);

    reset_levels();
}

LOV_TEST_MAIN()
