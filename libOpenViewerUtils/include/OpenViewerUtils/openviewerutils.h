// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#include "spdlog/spdlog.h"

// Compiler detection helper macro
#if defined(_MSC_VER)
#define LOVU_MSVC
#elif defined(__GNUC__)
#define LOVU_GCC
#elif defined(__clang__)
#define LOVU_CLANG
#endif

// Version and platform macro helpers
#if !defined(LOVU_VERSION_STR)
#define LOVU_VERSION_STR "Debug"
#endif

#include <cstdint>

#if INTPTR_MAX == INT64_MAX
#define LOVU_X64 1
#elif INTPTR_MAX == INT32_MAX
#define LOVU_X86 1
#endif

#if !defined(LOVU_PLATFORM_STR)
#if defined(_WIN32)
#define LOVU_WIN 1
#if defined(LOVU_X64)
#define LOVU_PLATFORM_STR "WIN64"
#else
#define LOVU_PLATFORM_STR "WIN32"
#endif
#elif defined(__linux__)
#define LOVU_LINUX 1
#if defined(LOVU_X64)
#define LOVU_PLATFORM_STR "LINUX64"
#else
#define LOVU_PLATFORM_STR "LINUX32"
#endif
#endif
#endif

// Function helper macros
#define LOVU_STATIC_FUNC static

#if defined(LOVU_WIN)
#if defined(LOVU_MSVC)
#define LOVU_EXPORT __declspec(dllexport)
#define LOVU_IMPORT __declspec(dllimport)
#elif defined(LOVU_GCC) || defined(LOVU_CLANG)
#define LOVU_EXPORT __attribute__((dllexport))
#define LOVU_IMPORT __attribute__((dllimport))
#endif 
#elif defined(LOVU_LINUX)
#define LOVU_EXPORT __attribute__((visibility("default")))
#define LOVU_IMPORT
#endif 

#if defined(LOVU_BUILD_EXPORT)
#define LOVU_API LOVU_EXPORT
#else
#define LOVU_API LOVU_IMPORT
#endif

#if defined(LOVU_MSVC) 
#define LOVU_FORCEINLINE __forceinline
#elif defined(LOVU_GCC)
#define LOVU_FORCEINLINE __attribute__((always_inline)) inline
#endif

// Logging helper macro
#define STRINGIFY(x) #x

#define GET_VARNAME(var) (#var)

#define DEBUG_VAR(var) spdlog::info("{} = {}", #var, var)

#define SET_SPDLOG_FMT spdlog::set_pattern("[%l] %H:%M:%S:%e : %v")

#if defined(LOVU_MSVC)
#define __LOVUFUNCTION__ __FUNCSIG__
#elif defined(LOVU_GCC) || defined(LOVU_CLANG)
#define __LOVUFUNCTION__ __PRETTY_FUNCTION__
#endif

#define __CLASS__ STRINGIFY(std::remove_reference<decltype(classMacroImpl(this))>::type)

template<class T> T& classMacroImpl(const T* t);

// Assertion helper macro
#include <assert.h>

#if defined(_DEBUG)
#define LOVU_ASSERT(expression) if (!(expression)) { spdlog::critical("Assertion failed : \nFunction : %s\nFile : %s\nLine : %d\n", __LOVUFUNCTION__, __FILE__, __LINE__); abort(); }
#else
#define LOVU_ASSERT(expression)
#endif

// Array size helper macro
#define LOVUARRAYSIZE(array) ((sizeof(array)/sizeof(0[array])) / ((size_t)(!(sizeof(array) % sizeof(0[array])))))

// Static cast helper macro
#define LOVU_CAST(type, var) static_cast<type>(var)

// Bit set helper macro
#define LOVU_BIT(bit) 1 << bit

// lovu namespace helper macro
#define LOVU_NAMESPACE_BEGIN namespace lovu {
#define LOVU_NAMESPACE_END }

LOVU_NAMESPACE_BEGIN

// Exception handler helper
#if defined(LOVU_WIN)
#include "boost/stacktrace.hpp"
inline static LONG WINAPI exception_handler(PEXCEPTION_POINTERS p_exception_info)
{
    spdlog::error("Unhandled exception trapped (Code {}) : \n>>>>>>>>>>\n{}\n<<<<<<<<<<\nProgram will exit.",
                  p_exception_info->ExceptionRecord->ExceptionCode,
                  boost::stacktrace::to_string(boost::stacktrace::stacktrace()));

    return EXCEPTION_CONTINUE_SEARCH;
}
#else
inline static void exception_handler() 
{

}
#endif

LOVU_FORCEINLINE uint32_t round_u32_to_next_pow2(uint32_t x) noexcept 
{
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x++;
}

LOVU_FORCEINLINE uint64_t round_u64_to_next_pow2(uint64_t x) noexcept 
{
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;
    return x++;
}

LOVU_NAMESPACE_END