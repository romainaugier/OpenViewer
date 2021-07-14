#pragma once

// header only utility to track memory usage
// by Max Liani Blog Post : https://maxliani.wordpress.com/2020/05/02/dev-tracking-memory-usage-part-1/

#ifndef MEM_TRACK
#define MEM_TRACK

#ifdef _WIN32

#include <windows.h>
#include <psapi.h>

inline size_t GetCurrentRss() noexcept
{
    // Obtain a handle to the current process, which is what we want to measure.
    // The process handle is not going to change, so we only do it once.
    static HANDLE process = GetCurrentProcess();

    PROCESS_MEMORY_COUNTERS counters;
    GetProcessMemoryInfo(process, &counters, sizeof(PROCESS_MEMORY_COUNTERS));
    return counters.WorkingSetSize;
}

inline size_t GetPeakRss() noexcept
{
    // Obtain a handle to the current process...
    static HANDLE process = GetCurrentProcess();

    PROCESS_MEMORY_COUNTERS counters;
    GetProcessMemoryInfo(process, &counters, sizeof(PROCESS_MEMORY_COUNTERS));
    return counters.PeakWorkingSetSize;
}

#elif __linux__

#include <sys/resource.h>
#include <unistd.h>
#include <stdio.h>

inline size_t GetCurrentRss() noexcept
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

inline size_t GetPeakRss() noexcept
{
    rusage usage_data;
    getrusage(RUSAGE_SELF, &usage_data);

    // From "man getrusage":
    //    ru_maxrss (since Linux 2.6.32)
    //    This is the maximum resident set size used (in kilobytes).
    // Compute the size in bytes.
    return size_t(usage_data.ru_maxrss) * 1024;
}

#elif __APPLE

#include <sys/resource.h>
#include <unistd.h>
#include <mach/mach.h>

inline size_t GetCurrentRss() noexcept
{
    // Configure what we want to query
    mach_task_basic_info info;
    mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;

    // Retrieve basic information about the process
    kern_return_t result = task_info(mach_task_self(), MACH_TASK_BASIC_INFO,
        (task_info_t)&info, &count);
    if (result != KERN_SUCCESS) return 0;

    return size_t(info.resident_size);
}

inline size_t GetPeakRss() noexcept
{
    rusage usage_data;
    getrusage(RUSAGE_SELF, &usage_data);

    // From "man getrusage":
    //    ru_maxrss the maximum resident set size utilized (in bytes).
    return size_t(usage_data.ru_maxrss);
}

#endif 

inline float ToMB(size_t size_in_bytes) noexcept
{
    return float(size_in_bytes) / float(1 << 20);
}

#endif // MEM_TRACK