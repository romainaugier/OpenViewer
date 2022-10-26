// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewerUtils/memory.h"
#include "immintrin.h"

LOVU_NAMESPACE_BEGIN

LOVU_DLL void* _mem_alloc(const size_t size, const size_t align) noexcept
{
    if(size == 0) return nullptr;

    void* tmp = _mm_malloc(size, align);
    
    if (tmp != nullptr)
    {
        return tmp;
    }
    else
    {
        spdlog::critical("Allocation failed. Exiting application");
        std::exit(EXIT_FAILURE);
    }
}

LOVU_DLL void _mem_free(void* ptr) noexcept
{
    if(ptr) _mm_free(ptr);
}

LOVU_DLL uint64_t get_total_system_memory() noexcept
{
#ifdef LOVU_WIN
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return status.ullTotalPhys;
#else if LOVU_LINUX
    int64_t pages = sysconf(_SC_PHYS_PAGES);
    int64_t page_size = sysconf(_SC_PAGE_SIZE);
    return pages * page_size;
#endif
}

#ifdef LOVU_WIN
LOVU_DLL size_t get_current_rss() noexcept
{
    // Obtain a handle to the current process, which is what we want to measure.
    // The process handle is not going to change, so we only do it once.
    static HANDLE process = GetCurrentProcess();

    PROCESS_MEMORY_COUNTERS counters;
    GetProcessMemoryInfo(process, &counters, sizeof(PROCESS_MEMORY_COUNTERS));
    return counters.WorkingSetSize;
}

LOVU_DLL size_t get_peak_rss() noexcept
{
    // Obtain a handle to the current process...
    static HANDLE process = GetCurrentProcess();

    PROCESS_MEMORY_COUNTERS counters;
    GetProcessMemoryInfo(process, &counters, sizeof(PROCESS_MEMORY_COUNTERS));
    return counters.PeakWorkingSetSize;
}

#elif LOVU_LINUX

LOVU_DLL size_t get_current_rss() noexcept
{
    // The value we query later on is measured in number of pages. Query the size
    // of a page in bytes. This is typically 4KB, but pages could be also configured
    // at the system level to be 2MB.
    static size_t page_size = sysconf(_SC_PAGESIZE);

    FILE* stat_file = fopen("/proc/self/statm", "r");
    if (!stat_file) return 0;

    // Attempt to read the value we need, it this won't succeed, size will be left
    // at zero.
    size_t pages_count = 0;
    fscanf(stat_file, "%ld %ld", &pages_count);
    fclose(stat_file);

    // Compute the size in bytes.
    return pages_count * page_size;
}

LOVU_DLL size_t get_peak_rss() noexcept
{
    rusage usage_data;
    getrusage(RUSAGE_SELF, &usage_data);

    // From "man getrusage":
    //    ru_maxrss (since Linux 2.6.32)
    //    This is the maximum resident set size used (in kilobytes).
    // Compute the size in bytes.
    return size_t(usage_data.ru_maxrss) * 1024;
}

#endif 
LOVU_FORCEINLINE float to_mb(size_t size_in_bytes) noexcept
{
    return float(size_in_bytes) / float(1 << 20);
}

LOVU_NAMESPACE_END