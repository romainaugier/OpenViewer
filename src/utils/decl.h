// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include "OpenEXR/half.h"

// Few macro utilities

#ifdef _MSC_VER
#define OV_MSVC 1
#define OV_FORCEINLINE __forceinline
#define OV_VECTORCALL __vectorcall
#define OV_FASTCALL __fastcall
#elif __GNUC__
#define OV_GCC 1
#define OV_FORCEINLINE __attribute__((always_inline)) inline
#define OV_VECTORCALL
#endif

#ifndef OV_VERSION_STR
#define OV_VERSION_STR "Debug"
#endif

#include <cstdint>

#if INTPTR_MAX == INT64_MAX
#define OV_X64 1
#elif INTPTR_MAX == INT32_MAX
#define OV_X86 1
#endif

#ifndef OV_PLATFORM_STR
#ifdef _WIN32
#define OV_WIN 1
#ifdef OV_X64
#define OV_PLATFORM_STR "WIN64"
#else
#define OV_PLATFORM_STR "WIN32"
#endif
#elif __linux__
#define OV_LINUX 1
#ifdef OV_X64
#define OV_PLATFORM_STR "LINUX64"
#else
#define OV_PLATFORM_STR "LINUX32"
#endif
#endif
#endif

#define OV_STATIC_FUNC static

#include <assert.h>

#ifdef OV_MSVC
#define __OVFUNCTION__ __FUNCSIG__
#else if OV_GCC
#define __OVFUNCTION__ __PRETTY_FUNCTION__
#endif

#ifdef _DEBUG
#define OVASSERT(expression) if (!expression) { printf("Assertion failed : \nFunction : %s\nFile : %s\nLine : %d\n", __OVFUNCTION__, __FILE__, __LINE__); abort(); }
#else
#define OVASSERT(expression)
#endif

// Borrowed from : https://stackoverflow.com/questions/4415524/common-array-length-macro-for-c
#define OVARRAYSIZE(array) ((sizeof(array)/sizeof(0[array])) / ((size_t)(!(sizeof(array) % sizeof(0[array])))))

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