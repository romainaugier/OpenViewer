// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__LOV_LOG)
#define __LOV_LOG

#include "OpenViewer/common.hpp"

#include <spdlog/spdlog.h>

#define LOG_NAMESPACE_BEGIN namespace log {
#define LOG_NAMESPACE_END }

LOV_NAMESPACE_BEGIN

LOG_NAMESPACE_BEGIN

LOV_API void initialize(spdlog::level::level_enum level = spdlog::level::info) noexcept;

LOG_NAMESPACE_END

LOV_NAMESPACE_END

#endif // !defined(__LOV_LOG)
