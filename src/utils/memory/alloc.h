#pragma once

#include "stdio.h"
#include <new>
#include "utils/decl.h"

template <typename t>
OPENVIEWER_FORCEINLINE t* aligned_alloc(size_t size, size_t alignement)
{
    return new (std::align_val_t(alignement)) t[size];
}

template <typename t>
OPENVIEWER_FORCEINLINE void aligned_free(t* ptr, size_t alignement)
{
    ::operator delete[](ptr, std::align_val_t(alignement));
}