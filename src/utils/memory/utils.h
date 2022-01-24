// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include "utils/decl.h"

namespace Utils
{
#ifdef OV_WIN
#include <windows.h>
    OV_STATIC_FUNC uint64_t GetTotalSystemMemory()
    {
        MEMORYSTATUSEX status;
        status.dwLength = sizeof(status);
        GlobalMemoryStatusEx(&status);
        return status.ullTotalPhys;
    }
#else if OV_LINUX
#include <unistd.h>
    OV_STATIC_FUNC uint64_t GetTotalSystemMemory()
    {
        int64_t pages = sysconf(_SC_PHYS_PAGES);
        int64_t page_size = sysconf(_SC_PAGE_SIZE);
        return pages * page_size;
    }
#endif

    OV_STATIC_FUNC OV_FORCEINLINE uint64_t MegaToBytes(const uint64_t megabytes) noexcept
    {
        return megabytes * 1000000;
    }
    
    OV_STATIC_FUNC OV_FORCEINLINE uint64_t BytesToMega(const uint64_t bytes) noexcept
    {
        return bytes / 1000000;
    }
} // End namespace Utils