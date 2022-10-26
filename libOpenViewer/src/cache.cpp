// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/cache.h"
#include "OpenViewerUtils/memory.h"

LOV_NAMESPACE_BEGIN

Cache::Cache(uint64_t size) 
{
    const uint64_t max_ram_capacity = lovu::get_total_system_memory();

    if(size >= max_ram_capacity)
    {
        size = max_ram_capacity / 2;

        spdlog::error("[CACHE] : Requested size is larger than available physical ram, setting cache size to {} bytes.",
                      size);
    }
    
    spdlog::debug("[CACHE] : Initializing cache ({} bytes)", size);

    m_bytes_capacity = size;

    m_memory_arena = lovu::mem_alloc(size, 32);
}

Cache::~Cache()
{
    if(m_memory_arena != nullptr)
    {
        lovu::mem_free(m_memory_arena);

        m_memory_arena = nullptr;
    }

    flush();

    spdlog::debug("[CACHE] : Released cache");
}

uint32_t add(const uint32_t hash, Media* item, const uint32_t frame) noexcept;

void Cache::remove(const uint32_t hash) noexcept
{

}

void Cache::flush() noexcept
{

}

void Cache::resize(const size_t new_size) noexcept
{

}

LOV_NAMESPACE_END