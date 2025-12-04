// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__LOV)
#define __LOV

#if defined(_MSC_VER)
#define LOV_MSVC
#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#elif defined(__GNUC__)
#define LOV_GCC
#elif defined(__clang__)
#define LOV_CLANG
#endif /* defined(_MSC_VER) */

#define LOV_STRIFY(x) #x
#define LOV_STRIFY_MACRO(m) LOV_STRIFY(m)

#if !defined(LOV_VERSION_MAJOR)
#define LOV_VERSION_MAJOR 0
#endif /* !defined(LOV_VERSION_MAJOR) */

#if !defined(LOV_VERSION_MINOR)
#define LOV_VERSION_MINOR 0
#endif /* !defined(LOV_VERSION_MINOR) */

#if !defined(LOV_VERSION_PATCH)
#define LOV_VERSION_PATCH 0
#endif /* !defined(LOV_VERSION_PATCH) */

#if !defined(LOV_VERSION_REVISION)
#define LOV_VERSION_REVISION 0
#endif /* !defined(LOV_VERSION_REVISION) */

#define LOV_VERSION_STR                                                                            \
    LOV_STRIFY_MACRO(LOV_VERSION_MAJOR)                                                            \
    "." LOV_STRIFY_MACRO(LOV_VERSION_MINOR) "." LOV_STRIFY_MACRO(                                  \
        LOV_VERSION_PATCH) "." LOV_STRIFY_MACRO(LOV_VERSION_REVISION)

#include <cassert>
#include <cstddef>
#include <cstdint>

#if INTPTR_MAX == INT64_MAX || defined(__x86_64__)
#define LOV_X64
#define LOV_SIZEOF_PTR 8
#elif INTPTR_MAX == INT32_MAX
#define LOV_X86
#define LOV_SIZEOF_PTR 4
#endif /* INTPTR_MAX == INT64_MAX || defined(__x86_64__) */

#if defined(_WIN32)
#define LOV_WIN
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif /* !defined(WIN32_LEAN_AND_MEAN) */
#if !defined(NOMINMAX)
#define NOMINMAX
#endif /* !defined(NOMINMAX) */
#if defined(LOV_X64)
#define LOV_PLATFORM_STR "WIN64"
#else
#define LOV_PLATFORM_STR "WIN32"
#endif /* defined(LOV_x64) */
#elif defined(__linux__)
#define LOV_LINUX
#if defined(LOV_X64)
#define LOV_PLATFORM_STR "LINUX64"
#else
#define LOV_PLATFORM_STR "LINUX32"
#endif /* defined(LOV_X64) */
#endif /* defined(_WIN32) */

#if defined(LOV_WIN)
#if defined(LOV_MSVC)
#define LOV_EXPORT __declspec(dllexport)
#define LOV_IMPORT __declspec(dllimport)
#elif defined(LOV_GCC) || defined(LOV_CLANG)
#define LOV_EXPORT __attribute__((dllexport))
#define LOV_IMPORT __attribute__((dllimport))
#endif /* defined(LOV_MSVC) */
#elif defined(LOV_LINUX)
#define LOV_EXPORT __attribute__((visibility("default")))
#define LOV_IMPORT
#endif /* defined(LOV_WIN) */

#if defined(LOV_MSVC)
#define LOV_FORCE_INLINE __forceinline
#define LOV_LIB_ENTRY
#define LOV_LIB_EXIT
#elif defined(LOV_GCC)
#define LOV_FORCE_INLINE inline __attribute__((always_inline))
#define LOV_LIB_ENTRY __attribute__((constructor))
#define LOV_LIB_EXIT __attribute__((destructor))
#elif defined(LOV_CLANG)
#define LOV_FORCE_INLINE __attribute__((always_inline))
#define LOV_LIB_ENTRY __attribute__((constructor))
#define LOV_LIB_EXIT __attribute__((destructor))
#endif /* defined(LOV_MSVC) */

#if defined(LOV_BUILD_SHARED)
#define LOV_API LOV_EXPORT
#define LOV_EXPIMP_TEMPLATE
#else
#define LOV_API LOV_IMPORT
#define LOV_EXPIMP_TEMPLATE extern
#endif /* defined(LOV_BUILD_SHARED) */

#if defined __cplusplus
#define LOV_CPP_ENTER                                                                              \
    extern "C"                                                                                     \
    {
#define LOV_CPP_END }
#else
#define LOV_CPP_ENTER
#define LOV_CPP_END
#endif /* DEFINED __cplusplus */

#if !defined NULL
#define NULL (void*)0
#endif /* !defined NULL */

#if defined(LOV_WIN)
#define LOV_FUNCTION __FUNCTION__
#elif defined(LOV_GCC) || defined(LOV_CLANG)
#define LOV_FUNCTION __PRETTY_FUNCTION__
#endif /* LOV_WIN */

#define CONCAT_(prefix, suffix) prefix##suffix
#define CONCAT(prefix, suffix) CONCAT_(prefix, suffix)

#define LOV_ASSERT(expr, message)                                                                  \
    if(!(expr))                                                                                    \
    {                                                                                              \
        std::fprintf(stderr,                                                                       \
                     "Assertion failed in file %s at line %d: %s\n",                               \
                     __FILE__,                                                                     \
                     __LINE__,                                                                     \
                     message);                                                                     \
        std::abort();                                                                              \
    }

#define LOV_STATIC_ASSERT(expr, message) static_assert(expr, message)
#define LOV_NOT_IMPLEMENTED                                                                        \
    std::fprintf(stderr,                                                                           \
                 "Called function %s that is not implemented (%s:%d)\n",                           \
                 LOV_FUNCTION,                                                                     \
                 __FILE__,                                                                         \
                 __LINE__);                                                                        \
    std::exit(1)

#define LOV_NON_COPYABLE(__class__)                                                                \
    __class__(const __class__&) = delete;                                                          \
    __class__(__class__&&) = delete;                                                               \
    const __class__& operator=(const __class__&) = delete;                                         \
    void operator=(__class__&&) = delete;

#if defined(LOV_MSVC)
#define LOV_PACKED_STRUCT(__struct__) __pragma(pack(push, 1)) __struct__ __pragma(pack(pop))
#elif defined(LOV_GCC) || defined(LOV_CLANG)
#define LOV_PACKED_STRUCT(__struct__) __struct__ __attribute__((__packed__))
#else
#define LOV_PACKED_STRUCT(__struct__) __struct__
#endif /* defined(LOV_MSVC) */

#define LOV_NO_DISCARD [[nodiscard]]
#define LOV_MAYBE_UNUSED [[maybe_unused]]
#define LOV_UNUSED(p) ((void)p)

#if defined(LOV_MSVC)
#define dump_struct(s)
#elif defined(LOV_CLANG)
#define dump_struct(s) __builtin_dump_struct(s, printf)
#elif defined(LOV_GCC)
#define dump_struct(s)
#endif /* defined(LOV_MSVC) */

#if defined(DEBUG_BUILD)
#define LOV_DEBUG 1
#else
#define LOV_DEBUG 0
#endif /* defined(DEBUG_BUILD) */

#define LOV_NAMESPACE_BEGIN                                                                        \
    namespace LOV                                                                                  \
    {
#define LOV_NAMESPACE_END }

#define DETAIL_NAMESPACE_BEGIN                                                                     \
    namespace detail                                                                               \
    {
#define DETAIL_NAMESPACE_END }

#define LOV_ATEXIT_REGISTER(func, do_exit)                                                         \
    int res_##func = std::atexit(func);                                                            \
    if(res_##func != 0)                                                                            \
    {                                                                                              \
        std::fprintf(stderr, "Cannot register function \"" #func "\" in atexit");                  \
        if(do_exit)                                                                                \
            std::exit(1);                                                                          \
    }

#endif /* !defined(__LOV) */
