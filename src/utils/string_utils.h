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
#include <regex>

#include "decl.h"

#include "OpenImageIO/imageio.h"

// #ifdef OPENVIEWER_MSVC
// #include <string_view>
// #elif OPENVIEWER_GCC
// #include "string_view"
// #endif

namespace Utils
{
    namespace Str
    {
        OPENVIEWER_STATIC_FUNC OPENVIEWER_FORCEINLINE bool EndsWith(const std::string& str, const std::string& suffix) noexcept
        {
            return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
        }

        OPENVIEWER_STATIC_FUNC OPENVIEWER_FORCEINLINE bool StartsWith(const std::string& str, const std::string& prefix) noexcept
        {
            return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
        }

        OPENVIEWER_STATIC_FUNC void RemoveSpaces(std::string& s) noexcept
        {
            char space = ' ';

            if ((char)s[0] == space) s.erase(0, 1);
            if ((char)s[s.size() - 1] == space) s.erase(s.size() - 1);
        }

        OPENVIEWER_STATIC_FUNC OPENVIEWER_FORCEINLINE void Replace(std::string& string, char substr, char replace) noexcept
        {
            std::replace(string.begin(), string.end(), substr, replace);
        }
        
        OPENVIEWER_STATIC_FUNC OPENVIEWER_FORCEINLINE void ReReplace(std::string& string, const std::regex& pattern, const std::string& replace) noexcept
        {
            string = std::regex_replace(string, pattern, replace);
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

        OPENVIEWER_STATIC_FUNC void Format(char* buffer, const char* fmt, ...) noexcept
        {
            va_list args;
            va_start(args, fmt);
            vsnprintf(buffer, 8192, fmt, args);
            va_end(args);
        }

        OPENVIEWER_STATIC_FUNC OPENVIEWER_FORCEINLINE std::string GetOIIOVersionStr() noexcept
        {
            char versionBuffer[32];
            Format(versionBuffer, "%d", OIIO::openimageio_version());
            std::string versionAsStr(versionBuffer);
            Replace(versionAsStr, '0', '.');
            return versionAsStr;
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
    } // End namespace Str
} // End namespace Utils