// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include "OpenEXR/half.h"

#ifdef _MSC_VER
#define OPENVIEWER_MSVC 1
#define OPENVIEWER_FORCEINLINE __forceinline
#define OPENVIEWER_VECTORCALL __vectorcall
#elif __GNUC__
#define OPENVIEWER_GCC 1
#define OPENVIEWER_FORCEINLINE __attribute__((always_inline))
#define OPENVIEWER_VECTORCALL
#endif

#ifndef OPENVIEWER_VERSION_STR
#define OPENVIEWER_VERSION_STR "Non-Official Version"
#endif

#ifndef OPENVIEWER_PLATFORM_STR
#ifdef _WIN32 || _WIN64
#ifdef _WIN64
#define OPENVIEWER_PLATFORM_STR "Windows x64"
#define OPENVIEWER_X64 1
#else
#define OPENVIEWER_VERSION_STR "Windows x86"
#define OPENVIEWER_x86 1
#endif
#endif
#elif __GNUC__
#ifdef __x86_64__ || __ppc64__
#define OPENVIEWER_PLATFORM_STR "Linux x64"
#define OPENVIEWER_X64 1
#else
#define OPENVIEWER_PLATFORM_STR "Linux x86"
#define OPENVIEWER_x86 1
#endif
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