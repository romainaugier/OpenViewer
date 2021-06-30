// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#pragma once

#ifdef _MSC_VER
#include <string_view>
#elif __GNUC__
#include "string_view"
#endif

bool endsWith(std::string_view str, std::string_view suffix);

bool startsWith(std::string_view str, std::string_view prefix);

std::string remove_spaces(char* s);