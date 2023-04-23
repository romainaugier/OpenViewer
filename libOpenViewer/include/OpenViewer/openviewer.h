// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#include "OpenViewerUtils/openviewerutils.h"

#define LOV_FORCEINLINE LOVU_FORCEINLINE

#if !defined(LOV_VERSION_STR)
#define LOV_VERSION_STR "Debug"
#endif

#if !defined(LOV_PLATFORM_STR)
#define LOV_PLATFORM_STR ""
#endif

#include <cstdint>

#if defined(LOV_BUILD_EXPORT)
#define LOV_API LOVU_EXPORT
#else
#define LOV_API LOVU_IMPORT
#endif

#define LOV_NAMESPACE_BEGIN namespace lov {
#define LOV_NAMESPACE_END }
