// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#include "OpenViewer/openviewer.h"

#define LOVA_FORCEINLINE LOVU_FORCEINLINE

#if !defined(LOVA_VERSION_STR)
#define LOVA_VERSION_STR "Debug"
#endif

#if !defined(LOVA_PLATFORM_STR)
#define LOVA_PLATFORM_STR ""
#endif

#if defined(LOVA_BUILD_EXPORT)
#define LOVA_API LOVU_EXPORT
#else
#define LOVA_API LOVU_IMPORT
#endif

#define LOVA_NAMESPACE_BEGIN namespace lova {
#define LOVA_NAMESPACE_END }
