// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/media_cache.hpp"
#include "OpenViewer/log.hpp"

int main() noexcept
{
    lov::log::initialize(spdlog::level::trace);

    lov::MediaCache cache(1024 * 1024 * 100); // 100MB cache

    void* frame1 = cache.allocate(1920 * 1080 * 4, []() -> void {
        spdlog::debug("Freeing initial frame");
    });

    for(std::size_t i = 0; i < 100; i++)
    {
        spdlog::debug("Adding frame {}", i);

        void* frame2 = cache.allocate(1920 * 1080 * 3, [=]() -> void {
            spdlog::debug("Freeing frame {}", i);
        });
    }

    return 0;
}
