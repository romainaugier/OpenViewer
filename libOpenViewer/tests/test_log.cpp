// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "fuzz_targets.hpp"
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

STDROMANO_TEST_CASE(category_names_are_explicit)
{
    const char* expected[] = {"media", "media_cache", "media_pool", "image_reader", "app"};

    STDROMANO_REQUIRE(sizeof(expected) / sizeof(expected[0]) == NUM_CATEGORIES);

    std::set<std::string> seen;

    for(std::size_t i = 0; i < NUM_CATEGORIES; ++i)
    {
        const LogCategory category = static_cast<LogCategory>(i);

        STDROMANO_CHECK_EQ(std::string(log::category_name(category)), std::string(expected[i]));
        STDROMANO_CHECK_EQ(log::get(category).name(), std::string(expected[i]));

        seen.insert(log::category_name(category));
    }

    STDROMANO_CHECK_EQ(seen.size(), NUM_CATEGORIES);
}

STDROMANO_TEST_CASE(every_category_is_named_in_the_file)
{
    reset_levels();

    for(std::size_t i = 0; i < NUM_CATEGORIES; ++i)
        log_info(static_cast<LogCategory>(i), "category_marker_{}", i);

    const std::string log = lov_test::read_log_file();

    for(std::size_t i = 0; i < NUM_CATEGORIES; ++i)
    {
        const std::string prefix = std::string("[ov::") + log::category_name(static_cast<LogCategory>(i)) + "]";

        STDROMANO_CHECK(lov_test::log_has_line(log, prefix, "category_marker_" + std::to_string(i)));
    }
}

STDROMANO_TEST_CASE(levels_are_per_category)
{
    reset_levels();

    log::set_level(LogCategory::MediaCache, spdlog::level::off);

    log_warn(LogCategory::MediaCache, "silenced_cache_marker");
    log_warn(LogCategory::Media, "audible_media_marker");

    const std::string log = lov_test::read_log_file();

    STDROMANO_CHECK(log.find("silenced_cache_marker") == std::string::npos);
    STDROMANO_CHECK(lov_test::log_has_line(log, "[ov::media]", "audible_media_marker"));

    reset_levels();
}

STDROMANO_TEST_CASE(apply_levels_accepts_valid_specs)
{
    reset_levels();

    STDROMANO_CHECK(log::apply_levels("media_cache=trace,image_reader=debug"));
    STDROMANO_CHECK_EQ(log::get(LogCategory::MediaCache).level(), spdlog::level::trace);
    STDROMANO_CHECK_EQ(log::get(LogCategory::ImageReader).level(), spdlog::level::debug);
    STDROMANO_CHECK_EQ(log::get(LogCategory::Media).level(), spdlog::level::info);

    // A bare level applies to every category, later entries override it
    STDROMANO_CHECK(log::apply_levels("warn, media = Debug"));
    STDROMANO_CHECK_EQ(log::get(LogCategory::MediaCache).level(), spdlog::level::warn);
    STDROMANO_CHECK_EQ(log::get(LogCategory::Media).level(), spdlog::level::debug);

    STDROMANO_CHECK(log::apply_levels(""));

    reset_levels();
}

// A typo must not silently change anything: spdlog's own from_str maps unknown
// names to "off", which would disable logging for that category
STDROMANO_TEST_CASE(apply_levels_rejects_typos_atomically)
{
    reset_levels();

    STDROMANO_CHECK(!log::apply_levels("mediacache=trace"));
    STDROMANO_CHECK(!log::apply_levels("media_cache=verbose"));
    STDROMANO_CHECK(!log::apply_levels("loud"));
    STDROMANO_CHECK(!log::apply_levels(nullptr));

    // The valid first entry must not be applied when the second one is wrong
    STDROMANO_CHECK(!log::apply_levels("media=trace,media_cache=verbose"));

    for(std::size_t i = 0; i < NUM_CATEGORIES; ++i)
        STDROMANO_CHECK_EQ(log::get(static_cast<LogCategory>(i)).level(), spdlog::level::info);
}

STDROMANO_TEST_CASE(environment_variable_is_applied)
{
    set_env("OPENVIEWER_LOG", "media_cache=trace");
    log::initialize(spdlog::level::warn);

    STDROMANO_CHECK_EQ(log::get(LogCategory::MediaCache).level(), spdlog::level::trace);
    STDROMANO_CHECK_EQ(log::get(LogCategory::Media).level(), spdlog::level::warn);

    // Invalid: ignored with a message, the level passed to initialize() stands
    set_env("OPENVIEWER_LOG", "media_cache=nope");
    log::initialize(spdlog::level::warn);

    STDROMANO_CHECK_EQ(log::get(LogCategory::MediaCache).level(), spdlog::level::warn);

    unset_env("OPENVIEWER_LOG");
    reset_levels();
}

// Loggers work before initialize() and after shutdown() (console only), and
// initializing again adds the file sink once, not twice
STDROMANO_TEST_CASE(reinitializing_does_not_duplicate_the_file)
{
    log::shutdown();

    log_warn(LogCategory::Media, "logged_while_shut_down");

    log::initialize(spdlog::level::info);

    log_warn(LogCategory::Media, "logged_once_marker");

    const std::string log = lov_test::read_log_file();

    STDROMANO_CHECK_EQ(count(log, "logged_once_marker"), std::size_t(1));
    STDROMANO_CHECK(log.find("logged_while_shut_down") == std::string::npos);

    reset_levels();
}

STDROMANO_TEST_CASE(fuzz_apply_levels_matches_model)
{
    const auto report = stdromano::fuzz::run_property(lov_test::fuzz_options("apply_levels_matches_model", 2000),
                                                      lov_fuzz::apply_levels_matches_model);

    LOV_REQUIRE_PROPERTY(report);
}

STDROMANO_TEST_CASE(fuzz_refused_specs_change_nothing)
{
    auto options = lov_test::fuzz_options("refused_specs_change_nothing", 3000);
    options.max_input_size = 256;
    options.dictionary = lov_fuzz::log_spec_dictionary();

    const auto to_bytes = [](const char* spec) {
        return std::vector<std::uint8_t>(spec, spec + std::strlen(spec));
    };

    options.corpus = {to_bytes("media_cache=trace,image_reader=debug"),
                      to_bytes("warn, media = Debug"),
                      to_bytes("media=trace,media_cache=verbose")};

    const auto report = stdromano::fuzz::run_input(options, lov_fuzz::apply_levels_any);

    LOV_REQUIRE_PROPERTY(report);
}

LOV_TEST_MAIN()
