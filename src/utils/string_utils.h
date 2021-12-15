// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#pragma once

#include <string>

#ifdef _MSC_VER
#include <string_view>
#elif __GNUC__
#include "string_view"
#endif

bool endsWith(const std::string_view& str, const std::string_view& suffix);

bool startsWith(const std::string_view& str, const std::string_view& prefix);

const char* remove_spaces(char* s);