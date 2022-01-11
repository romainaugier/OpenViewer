// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include "decl.h"

#ifdef OPENVIEWER_WIN

#include <windows.h>

uint64_t getTotalSystemMemory()
{
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return status.ullTotalPhys;
}

#else if OPENVIEWER_LINUX

#include <unistd.h>

uint64_t getTotalSystemMemory()
{
    int64_t pages = sysconf(_SC_PHYS_PAGES);
    int64_t page_size = sysconf(_SC_PAGE_SIZE);
    return pages * page_size;
}

#endif