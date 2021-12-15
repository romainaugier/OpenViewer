// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "string_utils.h"

namespace Utils
{
    bool EndsWith(const std::string_view& str, const std::string_view& suffix)
    {
        return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
    }

    bool StartsWith(const std::string_view& str, const std::string_view& prefix)
    {
        return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
    }

    const char* RemoveSpaces(char* s)
    {
        std::string tmp = s;

        char space[2] = " ";

        if ((char*)tmp[0] == space) tmp.erase(0, 1);
        if ((char*)tmp[tmp.size() - 1] == space) tmp.erase(tmp.size() - 1);

        return tmp.c_str();
    }
} // End namespace utils
