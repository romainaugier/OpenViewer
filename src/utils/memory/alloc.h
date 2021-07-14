// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#pragma once

#include "stdio.h"
#include "stdlib.h"
#include <new>
#include "utils/decl.h"
#include "utils/logger.h"

inline OPENVIEWER_FORCEINLINE void* OvAlloc(size_t size, size_t alignement)
{
    StaticDebugConsoleLog("[MEM_DEBUG] : Allocated %lld bytes.", size);
#ifdef __GNUC__
    return aligned_alloc(alignement, size);
#else _MSC_VER
    return static_cast<T>(_aligned_alloc(size, alignement));
#endif
}

inline OPENVIEWER_FORCEINLINE void OvFree(void* ptr)
{
    StaticDebugConsoleLog("[MEM_DEBUG] : Freed %d ptr.", ptr);
#ifdef __GNUC__
    free(ptr);
#else  _MSC_VER
    _aligned_free(ptr);
#endif
}