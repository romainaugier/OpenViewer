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

OV_FORCEINLINE void* OvAlloc(size_t size, size_t alignement)
{
    StaticDebugConsoleLog("[MEM_DEBUG] : Allocated %lld bytes.", size);
#ifdef OPENVIEWER_GCC
    void* tmp = aligned_alloc(alignement, size);
#else OPENVIEWER_MSVC
    void* tmp = _aligned_malloc(size, alignement);
#endif
    if (tmp != nullptr)
    {
        StaticDebugConsoleLog("[MEM_DEBUG] : 0x%p", tmp);
        return tmp;
    }
    else
    {
        StaticErrorConsoleLog("[MEMORY ERROR] : Allocation failed. Exiting application");
        std::exit(EXIT_FAILURE);
    }
}

OV_FORCEINLINE void OvFree(void* ptr)
{
    StaticDebugConsoleLog("[MEM_DEBUG] : Freed 0x%p ptr.", ptr);
#ifdef OPENVIEWER_GCC
    free(ptr);
#else OPENVIEWER_MSVC
    _aligned_free(ptr);
#endif
}