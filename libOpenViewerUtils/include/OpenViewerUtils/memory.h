// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#include "openviewerutils.h"
#include "spdlog/spdlog.h"

LOVU_NAMESPACE_BEGIN 

// Aligned memory allocation function
LOVU_DLL void* _mem_alloc(const size_t size, const size_t align) noexcept;

// Aligned memory free function
LOVU_DLL void _mem_free(void* ptr) noexcept;

// Helper macros to find where memory is allocated and freed
// To use them, define LOVU_MEM_DEBUG
#ifdef LOVU_MEM_DEBUG

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
LOVU_DLL uint64_t get_total_system_memory() noexcept;

// utilities to track memory usage
// by Max Liani Blog Post : https://maxliani.wordpress.com/2020/05/02/dev-tracking-memory-usage-part-1/

#ifdef LOVU_WIN

#include <windows.h>
#include <psapi.h>

// Get current memory usage of the application
LOVU_DLL size_t get_current_rss() noexcept;

// Get peak memory usage of the application
LOVU_DLL size_t get_peak_rss() noexcept;

#elif OV_LINUX

#include <sys/resource.h>
#include <unistd.h>
#include <stdio.h>

// Get current memory usage of the application
LOVU_DLL size_t get_current_rss() noexcept;

// Get peak memory usage of the application
LOVU_DLL size_t get_peak_rss() noexcept;

#endif 

// Converts the size from bytes to MegaBytes
LOVU_FORCEINLINE LOVU_DLL float to_mb(size_t size_in_bytes) noexcept;

LOVU_NAMESPACE_END