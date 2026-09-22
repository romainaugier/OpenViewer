// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "lov_test.hpp"

#include "OpenViewer/media_cache.hpp"

#include <atomic>
#include <memory>
#include <random>
#include <unordered_map>
#include <vector>

using namespace lov;

static constexpr std::size_t MB = 1024 * 1024;

LOV_TEST(allocate_within_capacity)
{
    MediaCache cache(16 * MB);

    LOV_CHECK_EQ(cache.get_capacity(), 16 * MB);

    void* block = cache.allocate(1 * MB);

    LOV_REQUIRE(block != nullptr);
    LOV_CHECK(cache.get_used_bytes() >= 1 * MB);

    std::memset(block, 0x5a, 1 * MB);

    cache.clear();
    LOV_CHECK_EQ(cache.get_used_bytes(), std::size_t(0));
}

LOV_TEST(eviction_runs_the_destructor)
{
    MediaCache cache(4 * MB);

    std::atomic<int> freed{0};

    // Twelve 1 MB blocks through a 4 MB cache so at least eight have to go
    for(int i = 0; i < 12; ++i)
    {
        void* block = cache.allocate(1 * MB, [&freed]() -> void { freed.fetch_add(1); });

        LOV_REQUIRE(block != nullptr);
        std::memset(block, i, 1 * MB);
    }

    LOV_CHECK(freed.load() >= 8);

    cache.clear();
}

LOV_TEST(oversized_allocation_is_refused)
{
    MediaCache cache(1 * MB);

    LOV_CHECK(cache.allocate(4 * MB) == nullptr);
}

LOV_TEST(headers_on_dirty_memory)
{
    MediaCache cache(4 * MB);

    void* big = cache.allocate(3 * MB);
    LOV_REQUIRE(big != nullptr);
    std::memset(big, 0xdf, 3 * MB);

    cache.clear();

    std::atomic<int> called{0};

    for(int i = 0; i < 64; ++i)
    {
        void* block = cache.allocate(64 * 1024, [&called]() -> void { called.fetch_add(1); });
        LOV_REQUIRE(block != nullptr);
        std::memset(block, 0xdf, 64 * 1024);
    }

    cache.clear();

    LOV_CHECK_EQ(called.load(), 64);
}

LOV_TEST(callbacks_are_destroyed)
{
    auto token = std::make_shared<int>(42);

    {
        MediaCache cache(1 * MB);

        for(int i = 0; i < 16; ++i)
            LOV_REQUIRE(cache.allocate(128 * 1024, [token]() -> void {}) != nullptr);

    }

    LOV_CHECK_EQ(token.use_count(), 1L);
}

LOV_TEST(randomized_ring_integrity)
{
    constexpr std::size_t CAPACITY = 2 * MB;

    std::mt19937 rng(1234);
    std::uniform_int_distribution<std::size_t> size_dist(1024, 700 * 1024);

    struct Live
    {
        unsigned char* data;
        std::size_t size;
    };

    for(int trial = 0; trial < 20; ++trial)
    {
        std::unordered_map<int, Live> live;

        MediaCache cache(CAPACITY);

        for(int id = 0; id < 400; ++id)
        {
            const std::size_t size = size_dist(rng);

            void* block = cache.allocate(size, [&live, id]() -> void { live.erase(id); });

            LOV_REQUIRE(block != nullptr);

            unsigned char* bytes = static_cast<unsigned char*>(block);
            std::memset(bytes, static_cast<unsigned char>(id & 0xff), size);

            live[id] = Live{bytes, size};

            // Nothing live may exceed what the cache claims to hold.
            std::size_t live_bytes = 0;

            for(const auto& entry : live)
            {
                live_bytes += entry.second.size;

                const unsigned char expected = static_cast<unsigned char>(entry.first & 0xff);
                const Live& l = entry.second;

                // First, middle and last byte: cheap, and an overlap from
                // either side touches one of them.
                LOV_REQUIRE(l.data[0] == expected);
                LOV_REQUIRE(l.data[l.size / 2] == expected);
                LOV_REQUIRE(l.data[l.size - 1] == expected);
            }

            LOV_REQUIRE(live_bytes <= CAPACITY);
            LOV_REQUIRE(cache.get_used_bytes() <= CAPACITY);
        }
    }
}

LOV_TEST_MAIN()
