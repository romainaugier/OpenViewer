// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include <string>
#include <algorithm>
#include <vector>
#include <sstream>
#include <stdio.h>
#include <stdarg.h>
#include "decl.h"

#ifdef OPENVIEWER_MSVC
#include <string_view>
#elif OPENVIEWER_GCC
#include "string_view"
#endif

namespace Utils
{
    OPENVIEWER_STATIC_FUNC OPENVIEWER_FORCEINLINE bool EndsWith(const std::string_view& str, const std::string_view& suffix) noexcept
    {
        return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
    }

    OPENVIEWER_STATIC_FUNC OPENVIEWER_FORCEINLINE bool StartsWith(const std::string_view& str, const std::string_view& prefix) noexcept
    {
        return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
    }

    OPENVIEWER_STATIC_FUNC const char* RemoveSpaces(char* s) noexcept
    {
        std::string tmp = s;

        char space[2] = " ";

        if ((char*)tmp[0] == space) tmp.erase(0, 1);
        if ((char*)tmp[tmp.size() - 1] == space) tmp.erase(tmp.size() - 1);

        return tmp.c_str();
    }

    OPENVIEWER_STATIC_FUNC OPENVIEWER_FORCEINLINE void Replace(std::string& string, const std::string& substr, const std::string& replace) noexcept
    {
        // std::replace(string.begin(), string.end(), substr.c_str(), replace.c_str());
    }

    OPENVIEWER_STATIC_FUNC OPENVIEWER_FORCEINLINE void CleanOSPath(std::string& string) noexcept
    {
        std::replace(string.begin(), string.end(), '\\', '/');
    }

    OPENVIEWER_STATIC_FUNC OPENVIEWER_FORCEINLINE std::string CleanOSPath(const std::string& string) noexcept
    {
        std::string newString = string;
        CleanOSPath(newString);
        return newString;
    }

    OPENVIEWER_STATIC_FUNC OPENVIEWER_FORCEINLINE void Split(std::vector<std::string>& outputStrings, const std::string& inputString, char delimiter) noexcept
    {
        std::string tmpString;

        std::istringstream stream(inputString);

        while (std::getline(stream, tmpString, delimiter))
        {
            outputStrings.push_back(tmpString);
        }
    }

    OPENVIEWER_STATIC_FUNC void Format(char* buffer, const char* fmt, ...) noexcept
    {
        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer, 8192, fmt, args);
        va_end(args);
    }
} // End namespace utils