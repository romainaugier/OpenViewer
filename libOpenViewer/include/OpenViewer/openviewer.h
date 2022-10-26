// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#include "spdlog/spdlog.h"
#include "OpenViewerUtils/openviewerutils.h"

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

#define LOV_DLL_EXPORT __declspec(dllexport)
#define LOV_DLL_IMPORT __declspec(dllimport)

#ifdef LOV_BUILD_DLL
#define LOV_DLL LOV_DLL_EXPORT
#else
#define LOV_DLL LOV_DLL_IMPORT
#endif

#define LOV_NAMESPACE_BEGIN namespace lov {
#define LOV_NAMESPACE_END }