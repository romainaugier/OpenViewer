// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Romain Augier
// All rights reserved.

#pragma once

#include <string>
#include <vector>
#include <regex>
#include <locale>
#include <codecvt>

#include "openviewerutils.h"

#include "fmt/core.h"

#if defined(LOVU_MSVC)
#include <string_view>
#elif defined(LOVU_GCC)
#include <string_view>
#endif

#define STR_NAMESPACE_BEGIN namespace str {
#define STR_NAMESPACE_END }

LOVU_NAMESPACE_BEGIN 

STR_NAMESPACE_BEGIN

// Tests if a string ends with a specific character
LOVU_FORCEINLINE bool ends_with(const std::string& str, const std::string& suffix) noexcept
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

LOVU_FORCEINLINE bool ends_with(const std::string_view& str, const std::string& suffix) noexcept
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

// Tests if a string starts with a specific character
LOVU_FORCEINLINE bool starts_with(const std::string& str, const std::string& prefix) noexcept
{
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

LOVU_FORCEINLINE bool starts_with(const std::string_view& str, const std::string& prefix) noexcept
{
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

// Removes the trailing spaces within a string
LOVU_API void strip(std::string& s) noexcept;

// Replaces a char occurence with another char within a string
LOVU_FORCEINLINE void replace(std::string& string, char substr, char replace) noexcept
{
    std::replace(string.begin(), string.end(), substr, replace);
}

// Replaces a regex pattern with another string within a string
LOVU_FORCEINLINE void re_replace(std::string& string, const std::regex& pattern, const std::string& replace) noexcept
{
    string = std::regex_replace(string, pattern, replace);
}

// Replaces any occurence of a \ with a /
LOVU_FORCEINLINE void clean_os_path(std::string& string) noexcept
{
    std::replace(string.begin(), string.end(), '\\', '/');
}

// Replaces any occurence of a \ with a /
LOVU_FORCEINLINE std::string clean_os_path(const std::string& string) noexcept
{
    std::string new_string = string;
    clean_os_path(new_string);
    return new_string;
}

// Splits a string and returns a vector containing each string separated by the specified delimiter
LOVU_API void split(std::vector<std::string>& output_strings, const std::string& input_string, char delimiter) noexcept;

STR_NAMESPACE_END

LOVU_NAMESPACE_END