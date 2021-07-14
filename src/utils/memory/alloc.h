// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#pragma once

#include "stdio.h"
#include "stdlib.h"
#include <new>
#include "utils/decl.h"

inline OPENVIEWER_FORCEINLINE void* OvAlloc(size_t size, size_t alignement)
{
    printf("[MEM_DEBUG] : Allocated %d bits of memory\n", size);
#ifdef __GNUC__
    return aligned_alloc(alignement, size);
#else _MSC_VER
    return static_cast<T>(_aligned_alloc(size, alignement));
#endif
}

inline OPENVIEWER_FORCEINLINE void OvFree(void* ptr)
{
#ifdef __GNUC__
    free(ptr);
#else  _MSC_VER
    _aligned_free(ptr);
#endif
}