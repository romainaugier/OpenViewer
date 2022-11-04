// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Romain Augier
// All rights reserved.

#include "OpenViewerUtils/string.h"

#include <sstream>

LOVU_NAMESPACE_BEGIN

STR_NAMESPACE_BEGIN

LOVU_DLL void strip(std::string& s) noexcept
{
    char space = ' ';

    if ((char)s[0] == space) s.erase(0, 1);
    if ((char)s[s.size() - 1] == space) s.erase(s.size() - 1);
}

LOVU_FORCEINLINE LOVU_DLL void split(std::vector<std::string>& output_strings, const std::string& input_string, char delimiter) noexcept
{
    std::string tmp_string;

    std::istringstream stream(input_string);

    while (std::getline(stream, tmp_string, delimiter))
    {
        output_strings.push_back(tmp_string);
    }
}
 
STR_NAMESPACE_END

LOVU_NAMESPACE_END