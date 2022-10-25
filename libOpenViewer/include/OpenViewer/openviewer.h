// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#include "OpenEXR/half.h"
#include "spdlog/spdlog.h"

// Few macro utilities

#ifdef _MSC_VER
#define LOV_MSVC 1
#define LOV_FORCEINLINE __forceinline
#elif __GNUC__
#define LOV_GCC 1
#define LOV_FORCEINLINE __attribute__((always_inline)) inline
#endif

#ifndef LOV_VERSION_STR
#define LOV_VERSION_STR "Debug"
#endif

#include <cstdint>

#if INTPTR_MAX == INT64_MAX
#define LOV_X64 1
#elif INTPTR_MAX == INT32_MAX
#define LOV_X86 1
#endif

#ifndef LOV_PLATFORM_STR
#ifdef _WIN32
#define LOV_WIN 1
#ifdef LOV_X64
#define LOV_PLATFORM_STR "WIN64"
#else
#define LOV_PLATFORM_STR "WIN32"
#endif
#elif __linux__
#define LOV_LINUX 1
#ifdef LOV_X64
#define LOV_PLATFORM_STR "LINUX64"
#else
#define LOV_PLATFORM_STR "LINUX32"
#endif
#endif
#endif

#define LOV_STATIC_FUNC static

#define LOV_DLL_EXPORT __declspec(dllexport)
#define LOV_DLL_IMPORT __declspec(dllimport)

#ifdef LOV_BUILD_DLL
#define LOV_DLL LOV_DLL_EXPORT
#else
#define LOV_DLL LOV_DLL_IMPORT
#endif

#include <assert.h>

#ifdef LOV_MSVC
#define __LOVFUNCTION__ __FUNCSIG__
#else if LOV_GCC
#define __LOVFUNCTION__ __PRETTY_FUNCTION__
#endif

#ifdef _DEBUG
#define LOV_ASSERT(expression) if (!(expression)) { spdlog::critical("Assertion failed : \nFunction : {}\nFile : {}\nLine : {}\n", __LOVFUNCTION__, __FILE__, __LINE__); abort(); }
#else
#define LOV_ASSERT(expression)
#endif

// Borrowed from : https://stackoverflow.com/questions/4415524/common-array-length-macro-for-c
#define LOVARRAYSIZE(array) ((sizeof(array)/sizeof(0[array])) / ((size_t)(!(sizeof(array) % sizeof(0[array])))))

#define CAST(type, var) static_cast<type>(var)

#define LOV_NAMESPACE_BEGIN namespace lov {
#define LOV_NAMESPACE_END }

#define SIZEOF_HALF_FLOAT sizeof(half);
#define SIZEOF_FLOAT sizeof(float);
#define SIZEOF_UINT8 sizeof(uint8_t);
#define SIZEOF_UINT16 sizeof(uint16_t)
#define SIZEOF_UINT32 sizeof(uint32_t)
#define SIZEOF_UINT64 sizeof(uint64_t)

#define SIZE_8_BITS 1
#define SIZE_16_BITS 2 
#define SIZE_32_BITS 4
#define SIZE_64_BITS 8