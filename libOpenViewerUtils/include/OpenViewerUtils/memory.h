// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#include "openviewerutils.h"
#include "spdlog/spdlog.h"

LOVU_NAMESPACE_BEGIN 

// Aligned memory allocation function
LOVU_API void* _mem_alloc(const size_t size, const size_t align) noexcept;

// Aligned memory free function
LOVU_API void _mem_free(void* ptr) noexcept;

// Helper macros to find where memory is allocated and freed
// To use them, define LOVU_MEM_DEBUG
#if defined(LOVU_MEM_DEBUG)

#define mem_alloc(size, align) do { \
    _mem_alloc(size, align); \
    spdlog::debug("[MEM DEBUG] : Allocated {} bytes (L:{}|F:{})", size, __LINE__, __FILE__); \
} while(0)

#define mem_free(ptr) do { \
    _mem_free(ptr);
    spdlog::debug("[MEM DEBUG] : Freed ptr {:p} (L:{}|F:{})", ptr, __LINE__, __FILE__); \
} while(0)

#else

#define mem_alloc(size, align) _mem_alloc(size, align)

#define mem_free(ptr) _mem_free(ptr)

#endif

// Returns the total amount of physical ram available
LOVU_API uint64_t get_total_system_memory() noexcept;

// utilities to track memory usage
// by Max Liani Blog Post : https://maxliani.wordpress.com/2020/05/02/dev-tracking-memory-usage-part-1/

#if defined(LOVU_WIN)

#include <windows.h>
#include <psapi.h>

// Get current memory usage of the application
LOVU_API size_t get_current_rss() noexcept;

// Get peak memory usage of the application
LOVU_API size_t get_peak_rss() noexcept;

#elif defined(LOVU_LINUX)

#include <sys/resource.h>
#include <unistd.h>
#include <stdio.h>

// Get current memory usage of the application
LOVU_API size_t get_current_rss() noexcept;

// Get peak memory usage of the application
LOVU_API size_t get_peak_rss() noexcept;

#endif 

// Converts the size from bytes to megabytes
LOVU_FORCEINLINE float to_mb(size_t size_in_bytes) noexcept { return float(size_in_bytes) / float(LOVU_BIT(20)); }

// Converts the size from megabytes to bytes
LOVU_FORCEINLINE float from_mb(size_t size_in_mbytes) noexcept { return float(size_in_mbytes) * float(LOVU_BIT(20)); }

// Converts the size from bytes to gigabytes
LOVU_FORCEINLINE float to_gb(size_t size_in_bytes) noexcept { return float(size_in_bytes) / float(LOVU_BIT(30)); }

// Converts the size from gigabytes to bytes
LOVU_FORCEINLINE float from_gb(size_t size_in_gbytes) noexcept { return float(size_in_gbytes) * float(LOVU_BIT(30)); }

LOVU_NAMESPACE_END