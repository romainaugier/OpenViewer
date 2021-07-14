#pragma once

#include "stdio.h"
#include "stdlib.h"
#include <new>
#include "utils/decl.h"

inline OPENVIEWER_FORCEINLINE void* OvAlloc(size_t size, size_t alignement)
{
#ifdef __GNUC__
    return aligned_alloc(alignement, size);
#else ifdef _MSC_VER
    return static_cast<T>(_aligned_alloc(size, alignement));
#endif
}

inline OPENVIEWER_FORCEINLINE void OvFree(void* ptr)
{
#ifdef __GNUC__
    free(ptr);
#else ifdef _MSC_VER
    _aligned_free(ptr);
#endif
}