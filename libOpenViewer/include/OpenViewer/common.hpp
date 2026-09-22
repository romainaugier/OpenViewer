// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__LOV)
#define __LOV

// Compiler detection
#if defined(__clang__)
#define LOV_CLANG
#elif defined(_MSC_VER)
#define LOV_MSVC
#elif defined(__GNUC__)
#define LOV_GCC
#else
#error "Unsupported compiler"
#endif // defined(__clang__)

#if defined(_MSC_VER)
#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif // defined(_MSC_VER)

#define LOV_STRIFY(x) #x
#define LOV_STRIFY_MACRO(m) LOV_STRIFY(m)

#if !defined(LOV_VERSION_MAJOR)
#define LOV_VERSION_MAJOR 0
#endif // !defined(LOV_VERSION_MAJOR)

#if !defined(LOV_VERSION_MINOR)
#define LOV_VERSION_MINOR 0
#endif // !defined(LOV_VERSION_MINOR)

#if !defined(LOV_VERSION_PATCH)
#define LOV_VERSION_PATCH 0
#endif // !defined(LOV_VERSION_PATCH)

#if !defined(LOV_VERSION_REVISION)
#define LOV_VERSION_REVISION 0
#endif // !defined(LOV_VERSION_REVISION)

#define LOV_VERSION_STR                                                                            \
    LOV_STRIFY_MACRO(LOV_VERSION_MAJOR)                                                            \
    "." LOV_STRIFY_MACRO(LOV_VERSION_MINOR) "." LOV_STRIFY_MACRO(                                  \
        LOV_VERSION_PATCH) "." LOV_STRIFY_MACRO(LOV_VERSION_REVISION)

#include <cassert>
#include <cstddef>
#include <cstdint>

// Architecture detection, kept in sync with stdromano.hpp
// https://github.com/cpredef/predef/blob/master/Architectures.md
#if defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64)
#define LOV_X86_64
#define LOV_SIZEOF_PTR 8
#define LOV_ARCH_STR "X86_64"
#elif defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
#define LOV_AARCH64
#define LOV_SIZEOF_PTR 8
#define LOV_ARCH_STR "AARCH64"
#else
/* 32 bits targets are not supported: a single 4K float frame is already past
   what a 32 bits address space can map comfortably. */
#error "Unsupported architecture, OpenViewer supports x86_64 and aarch64"
#endif /* defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64) */

#if defined(LOV_X86_64)
#define LOV_INTEL
#elif defined(LOV_AARCH64)
#define LOV_ARM
#endif // defined(LOV_X86_64)

// Operating system detection
#if defined(_WIN32)
#define LOV_WIN
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif // !defined(WIN32_LEAN_AND_MEAN)
#if !defined(NOMINMAX)
#define NOMINMAX
#endif // !defined(NOMINMAX)
#define LOV_OS_STR "WIN"
#elif defined(__linux__)
#define LOV_LINUX
#define LOV_UNIX
#define LOV_OS_STR "LINUX"
#elif defined(__APPLE__)
#define LOV_APPLE
#define LOV_UNIX
#define LOV_OS_STR "APPLE"
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
#define LOV_BSD
#define LOV_UNIX
#define LOV_OS_STR "BSD"
#else
#error "Unsupported platform"
#endif // defined(_WIN32)

#define LOV_PLATFORM_STR LOV_OS_STR "_" LOV_ARCH_STR

#if defined(LOV_WIN)
#if defined(LOV_MSVC)
#define LOV_EXPORT __declspec(dllexport)
#define LOV_IMPORT __declspec(dllimport)
#elif defined(LOV_GCC) || defined(LOV_CLANG)
#define LOV_EXPORT __attribute__((dllexport))
#define LOV_IMPORT __attribute__((dllimport))
#endif // defined(LOV_MSVC)
#elif defined(LOV_UNIX)
#define LOV_EXPORT __attribute__((visibility("default")))
#define LOV_IMPORT
#endif // defined(LOV_WIN)

#if defined(LOV_MSVC)
#define LOV_FORCE_INLINE __forceinline
#define LOV_LIB_ENTRY
#define LOV_LIB_EXIT
#elif defined(LOV_GCC)
#define LOV_FORCE_INLINE inline __attribute__((always_inline))
#define LOV_LIB_ENTRY __attribute__((constructor))
#define LOV_LIB_EXIT __attribute__((destructor))
#elif defined(LOV_CLANG)
#define LOV_FORCE_INLINE inline __attribute__((always_inline))
#define LOV_LIB_ENTRY __attribute__((constructor))
#define LOV_LIB_EXIT __attribute__((destructor))
#endif // defined(LOV_MSVC)

#if defined(LOV_BUILD_SHARED)
#define LOV_API LOV_EXPORT
#define LOV_EXPIMP_TEMPLATE
#else
#define LOV_API LOV_IMPORT
#define LOV_EXPIMP_TEMPLATE extern
#endif // defined(LOV_BUILD_SHARED)

#if defined(LOV_WIN)
#define LOV_FUNCTION __FUNCTION__
#elif defined(LOV_GCC) || defined(LOV_CLANG)
#define LOV_FUNCTION __func__
#endif // LOV_WIN

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

#define LOV_NOT_IMPLEMENTED                                                                        \
    std::fprintf(stderr,                                                                           \
                 "Called function %s that is not implemented (%s:%d)\n",                           \
                 LOV_FUNCTION,                                                                     \
                 __FILE__,                                                                         \
                 __LINE__);                                                                        \
    std::exit(1)

#define LOV_NON_COPYABLE(__class__)                                                                \
    __class__(const __class__&) = delete;                                                          \
    const __class__& operator=(const __class__&) = delete;

#define LOV_NON_MOVABLE(__class__)                                                                 \
    __class__(__class__&&) = delete;                                                               \
    void operator=(__class__&&) = delete;

#if defined(LOV_MSVC)
#define LOV_PACKED_STRUCT(__struct__) __pragma(pack(push, 1)) __struct__ __pragma(pack(pop))
#elif defined(LOV_GCC) || defined(LOV_CLANG)
#define LOV_PACKED_STRUCT(__struct__) __struct__ __attribute__((__packed__))
#else
#define LOV_PACKED_STRUCT(__struct__) __struct__
#endif // defined(LOV_MSVC)

#define LOV_NO_DISCARD [[nodiscard]]
#define LOV_MAYBE_UNUSED [[maybe_unused]]
#define LOV_UNUSED(p) ((void)p)

#if defined(LOV_MSVC)
#define dump_struct(s)
#elif defined(LOV_CLANG)
#define dump_struct(s) __builtin_dump_struct(s, printf)
#elif defined(LOV_GCC)
#define dump_struct(s)
#endif // defined(LOV_MSVC)

#if defined(DEBUG_BUILD)
#define LOV_DEBUG 1
#else
#define LOV_DEBUG 0
#endif // defined(DEBUG_BUILD)

#define LOV_NAMESPACE_BEGIN namespace lov {
#define LOV_NAMESPACE_END }

#define DETAIL_NAMESPACE_BEGIN namespace detail {
#define DETAIL_NAMESPACE_END }

#define LOV_ATEXIT_REGISTER(func, do_exit)                                                         \
    int res_##func = std::atexit(func);                                                            \
    if(res_##func != 0)                                                                            \
    {                                                                                              \
        std::fprintf(stderr, "Cannot register function \"" #func "\" in atexit");                  \
        if(do_exit)                                                                                \
            std::exit(1);                                                                          \
    }

#endif // !defined(__LOV)
