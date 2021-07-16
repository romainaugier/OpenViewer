// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#pragma once

#include "stdio.h"
#include "stdlib.h"
#include <cstdlib>
#include <new>
#include "utils/decl.h"
#include "utils/logger.h"

inline OPENVIEWER_FORCEINLINE void* OvAlloc(size_t size, size_t alignement)
{
    StaticDebugConsoleLog("[MEM_DEBUG] : Allocated %lld bytes.", size);
#ifdef __GNUC__
    void* tmp = aligned_alloc(alignement, size);
#else _MSC_VER
    void* tmp = _aligned_malloc(size, alignement);
#endif
    if (tmp != nullptr)
    {
        StaticDebugConsoleLog("[MEM_DEBUG] : 0x%p", tmp);
        return tmp;
    }
    else
    {
        StaticDebugConsoleLog("[MEMORY ERROR] : Allocation failed, OpenViewer will exit");
        exit(1);
    }
}

inline OPENVIEWER_FORCEINLINE void OvFree(void* ptr)
{
    StaticDebugConsoleLog("[MEM_DEBUG] : Freed 0x%p ptr.", ptr);
#ifdef __GNUC__
    free(ptr);
#else  _MSC_VER
    _aligned_free(ptr);
#endif
}