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

using ByteSize = uint8_t;

constexpr ByteSize size64 = 8;
constexpr ByteSize size32 = 4;
constexpr ByteSize size16 = 2;
constexpr ByteSize size8 = 1;