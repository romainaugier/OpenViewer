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

namespace Utils
{
    bool EndsWith(const std::string_view& str, const std::string_view& suffix);

    bool StartsWith(const std::string_view& str, const std::string_view& prefix);

    const char* RemoveSpaces(char* s);
} // End namespace utils