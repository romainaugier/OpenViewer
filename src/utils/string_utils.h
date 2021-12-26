// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include <string>
#include <algorithm>
#include "decl.h"

#ifdef _MSC_VER
#include <string_view>
#elif __GNUC__
#include "string_view"
#endif

namespace Utils
{
    OPENVIEWER_FORCEINLINE bool EndsWith(const std::string_view& str, const std::string_view& suffix)
    {
        return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
    }

    OPENVIEWER_FORCEINLINE bool StartsWith(const std::string_view& str, const std::string_view& prefix)
    {
        return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
    }

    OPENVIEWER_FORCEINLINE const char* RemoveSpaces(char* s)
    {
        std::string tmp = s;

        char space[2] = " ";

        if ((char*)tmp[0] == space) tmp.erase(0, 1);
        if ((char*)tmp[tmp.size() - 1] == space) tmp.erase(tmp.size() - 1);

        return tmp.c_str();
    }

    OPENVIEWER_FORCEINLINE void Replace(std::string& string, const std::string& substr, const std::string& replace)
    {
        // std::replace(string.begin(), string.end(), substr.c_str(), replace.c_str());
    }

    OPENVIEWER_FORCEINLINE void CleanOSPath(std::string& string)
    {
        std::replace(string.begin(), string.end(), '\\', '/');
    }

    OPENVIEWER_FORCEINLINE std::string CleanOSPath(const std::string& string)
    {
        std::string newString = string;
        CleanOSPath(newString);
        return newString;
    }
} // End namespace utils