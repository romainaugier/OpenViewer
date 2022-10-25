// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewerUtils/hash.h"

LOVU_NAMESPACE_BEGIN

uint32_t hash_fnv1a(char *str) noexcept
{
    uint32_t result = EMPTY_HASH;
    uint8_t* s = (uint8_t*)str;

    while (*s)
    {
        result ^= (uint32_t) tolower(*s); 
        result *= (uint32_t) 0x01000193;
        s++;
    }

    return result;
}

LOVU_NAMESPACE_END