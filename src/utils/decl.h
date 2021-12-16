// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include "OpenEXR/half.h"

#ifdef _MSC_VER
#define OPENVIEWER_FORCEINLINE __forceinline
#define OPENVIEWER_VECTORCALL __vectorcall
#elif __GNUC__
#define OPENVIEWER_FORCEINLINE __attribute__((always_inline))
#define OPENVIEWER_VECTORCALL
#endif

// utility constants
constexpr int SIZEOF_FLOAT = sizeof(float);
constexpr int SIZEOF_HALF_FLOAT = sizeof(half);
constexpr int SIZEOF_UINT8 = sizeof(uint8_t);

namespace Size
{
    using ByteSize = uint8_t;

    constexpr ByteSize Size64 = 8;
    constexpr ByteSize Size32 = 4; // Used for 32 bits int, float
    constexpr ByteSize Size16 = 2; // Used for half floats
    constexpr ByteSize Size8 = 1;  // Used for uint8_t, char, uchar, any 8 bits format
}