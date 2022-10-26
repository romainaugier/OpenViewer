// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#include "spdlog/spdlog.h"

// Few macro utilities

#ifdef _MSC_VER
#define LOVU_MSVC 1
#define LOVU_FORCEINLINE __forceinline
#elif __GNUC__
#define LOVU_GCC 1
#define LOVU_FORCEINLINE __attribute__((always_inline)) inline
#endif

#ifndef LOVU_VERSION_STR
#define LOVU_VERSION_STR "Debug"
#endif

#include <cstdint>

#if INTPTR_MAX == INT64_MAX
#define LOVU_X64 1
#elif INTPTR_MAX == INT32_MAX
#define LOVU_X86 1
#endif

#ifndef LOVU_PLATFORM_STR
#ifdef _WIN32
#define LOVU_WIN 1
#ifdef LOVU_X64
#define LOVU_PLATFORM_STR "WIN64"
#else
#define LOVU_PLATFORM_STR "WIN32"
#endif
#elif __linux__
#define LOVU_LINUX 1
#ifdef LOVU_X64
#define LOVU_PLATFORM_STR "LINUX64"
#else
#define LOVU_PLATFORM_STR "LINUX32"
#endif
#endif
#endif

#define LOVU_STATIC_FUNC static

#define LOVU_DLL_EXPORT __declspec(dllexport)
#define LOVU_DLL_IMPORT __declspec(dllimport)

#ifdef LOVU_BUILD_DLL
#define LOVU_DLL LOVU_DLL_EXPORT
#else
#define LOVU_DLL LOVU_DLL_IMPORT
#endif

#include <assert.h>

#ifdef LOVU_MSVC
#define __LOVUFUNCTION__ __FUNCSIG__
#else if LOVU_GCC
#define __LOVUFUNCTION__ __PRETTY_FUNCTION__
#endif

#ifdef _DEBUG
#define LOVU_ASSERT(expression) if (!(expression)) { spdlog::critical("Assertion failed : \nFunction : %s\nFile : %s\nLine : %d\n", __LOVUFUNCTION__, __FILE__, __LINE__); abort(); }
#else
#define LOVU_ASSERT(expression)
#endif

// Borrowed from : https://stackLOVUerflow.com/questions/4415524/common-array-length-macro-for-c
#define LOVUARRAYSIZE(array) ((sizeof(array)/sizeof(0[array])) / ((size_t)(!(sizeof(array) % sizeof(0[array])))))

#define CAST(type, var) static_cast<type>(var)

#define BIT(bit) 1 << bit

#define GET_VARNAME(var) (#var)
#define DEBUG_VAR(var) spdlog::info("{} = {}", #var, var)

#define STRINGIFY(x) #x

#define LOVU_NAMESPACE_BEGIN namespace lovu {
#define LOVU_NAMESPACE_END }