// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

// Reference: http://www.isthe.com/chongo/tech/comp/fnv/

#include "OpenViewerUtils/openviewerutils.h"

LOVU_NAMESPACE_BEGIN

#define EMPTY_HASH ((uint32_t)0x811c9dc5)

// 32 bits FNV 1A string hash
LOVU_API uint32_t hash_fnv1a(const char *str) noexcept;

// 32 bits string hash function
LOVU_FORCEINLINE uint32_t hash_string(const char* str) noexcept { return hash_fnv1a(str); }

LOVU_NAMESPACE_END