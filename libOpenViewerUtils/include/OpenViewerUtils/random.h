// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#if !defined(__LOVURANDOM)
#define __LOVURANDOM

#include "OpenViewerUtils/openviewerutils.h"

LOVU_NAMESPACE_BEGIN

LOVU_FORCEINLINE int wang_hash(int state) noexcept
{
    state = (state ^ 61u) ^ (state >> 16u);
    state *= 9u;
    state = state ^ (state >> 4u);
    state *= 0x27d4eb2du;
    state = state ^ (state >> 15u);
    return 1u + state;
}

LOVU_FORCEINLINE int xorshift32(int state) noexcept
{
    int x = state;
    x ^= x << 13u;
    x ^= x >> 17u;
    x ^= x << 5u;
    return x;
}

inline int wang_hash_sampler_32(int state) noexcept
{
    state = wang_hash(state);
    return xorshift32(state);
}

inline float wang_hash_sampler_f32(int state) noexcept
{
    constexpr unsigned int to_float = 0x2f800004u;

    state = wang_hash(state);
    state = xorshift32(state);

    return static_cast<float>(state) * reinterpret_cast<const float&>(to_float) + 0.5f;
}

LOVU_NAMESPACE_END

#endif // !defined(__LOVURANDOM)
