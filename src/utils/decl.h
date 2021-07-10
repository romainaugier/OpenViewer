#pragma once

#ifdef _MSC_VER
#define OPENVIEWER_FORCEINLINE __forceinline
#define OPENVIEWER_VECTORCALL __vectorcall
#elif __GNUC__
#define OPENVIEWER_FORCEINLINE __attribute__((always_inline))
#define OPENVIEWER_VECTORCALL
#endif