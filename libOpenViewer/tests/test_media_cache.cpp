// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "fuzz_targets.hpp"
#include "lov_test.hpp"

#include "OpenViewer/media_cache.hpp"

#include <atomic>
#include <memory>
#include <vector>

using namespace lov;

static constexpr std::size_t MB = 1024 * 1024;

STDROMANO_TEST_CASE(allocate_within_capacity)
{
    MediaCache cache(16 * MB);

    STDROMANO_CHECK_EQ(cache.get_capacity(), 16 * MB);

    void* block = cache.allocate(1 * MB);

    STDROMANO_REQUIRE_NE(block, nullptr);
    STDROMANO_CHECK(cache.get_used_bytes() >= 1 * MB);

    std::memset(block, 0x5a, 1 * MB);

    cache.clear();
    STDROMANO_CHECK_EQ(cache.get_used_bytes(), std::size_t(0));
}

STDROMANO_TEST_CASE(eviction_runs_the_destructor)
{
    MediaCache cache(4 * MB);

    std::atomic<int> freed{0};

    // Twelve 1 MB blocks through a 4 MB cache so at least eight have to go
    for(int i = 0; i < 12; ++i)
    {
        void* block = cache.allocate(1 * MB, [&freed]() -> void { freed.fetch_add(1); });

        STDROMANO_REQUIRE_NE(block, nullptr);
        std::memset(block, i, 1 * MB);
    }

    STDROMANO_CHECK(freed.load() >= 8);

    cache.clear();
}

STDROMANO_TEST_CASE(oversized_allocation_is_refused)
{
    MediaCache cache(1 * MB);

    STDROMANO_CHECK(cache.allocate(4 * MB) == nullptr);

    // Rounding such a size up to the block alignment wraps around to a small one
    STDROMANO_CHECK(cache.allocate(SIZE_MAX) == nullptr);
    STDROMANO_CHECK(cache.allocate(SIZE_MAX - 16) == nullptr);
}

STDROMANO_TEST_CASE(headers_on_dirty_memory)
{
    MediaCache cache(4 * MB);

    void* big = cache.allocate(3 * MB);
    STDROMANO_REQUIRE_NE(big, nullptr);
    std::memset(big, 0xdf, 3 * MB);

    cache.clear();

    std::atomic<int> called{0};

    for(int i = 0; i < 64; ++i)
    {
        void* block = cache.allocate(64 * 1024, [&called]() -> void { called.fetch_add(1); });
        STDROMANO_REQUIRE_NE(block, nullptr);
        std::memset(block, 0xdf, 64 * 1024);
    }

    cache.clear();

    STDROMANO_CHECK_EQ(called.load(), 64);
}

STDROMANO_TEST_CASE(callbacks_are_destroyed)
{
    auto token = std::make_shared<int>(42);

    {
        MediaCache cache(1 * MB);

        for(int i = 0; i < 16; ++i)
            STDROMANO_REQUIRE_NE(cache.allocate(128 * 1024, [token]() -> void {}), nullptr);

    }

    STDROMANO_CHECK_EQ(token.use_count(), 1L);
}

STDROMANO_TEST_CASE(fuzz_live_blocks_stay_intact)
{
    lov_test::QuietLogs quiet;

    const auto report = stdromano::fuzz::run_property(lov_test::fuzz_options("media_cache_live_blocks", 1000),
                                                      lov_fuzz::media_cache_keeps_live_blocks_intact);

    LOV_REQUIRE_PROPERTY(report);
}

// Sizes are formatted lazily, only when a message is written
STDROMANO_TEST_CASE(logs_sizes_in_readable_units)
{
    log::set_level(LogCategory::MediaCache, spdlog::level::trace);

    {
        MediaCache cache(4 * MB);
        STDROMANO_REQUIRE_NE(cache.allocate(1536 * 1024), nullptr);
    }

    log::set_level(LogCategory::MediaCache, spdlog::level::warn);

    const std::string log = lov_test::read_log_file();

    STDROMANO_CHECK(lov_test::log_has_line(log, "[ov::media_cache]", "| 1.57 Mb)"));

    // Trace calls are compiled out of release builds (LOV_LOG_ACTIVE_LEVEL)
#if LOV_LOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_TRACE
    STDROMANO_CHECK(lov_test::log_has_line(log, "[ov::media_cache]", "Requested a 1.57 Mb block"));
    STDROMANO_CHECK(lov_test::log_has_line(log, "[ov::media_cache]", "Initialized with 4.19 Mb"));
#else
    STDROMANO_CHECK(log.find("Requested a 1.57 Mb block") == std::string::npos);
#endif // LOV_LOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_TRACE
}

LOV_TEST_MAIN()
