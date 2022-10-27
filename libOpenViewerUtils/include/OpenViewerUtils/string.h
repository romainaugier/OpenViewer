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

#ifdef LOVU_MSVC
#include <string_view>
#elif LOVU_GCC
#include "string_view"
#endif

#define STR_NAMESPACE_BEGIN namespace str {
#define STR_NAMESPACE_END }

LOVU_NAMESPACE_BEGIN 

STR_NAMESPACE_BEGIN

// Tests if a string ends with a specific character
LOVU_FORCEINLINE LOVU_DLL bool ends_with(const std::string& str, const std::string& suffix) noexcept;
LOVU_FORCEINLINE LOVU_DLL bool ends_with(const std::string_view& str, const std::string& suffix) noexcept;

// Tests if a string starts with a specific character
LOVU_FORCEINLINE LOVU_DLL bool starts_with(const std::string& str, const std::string& prefix) noexcept;
LOVU_FORCEINLINE LOVU_DLL bool starts_with(const std::string_view& str, const std::string& prefix) noexcept;

// Removes the trailing spaces within a string
LOVU_DLL void strip(std::string& s) noexcept;

// Replaces a char occurence with another char within a string
LOVU_FORCEINLINE LOVU_DLL void replace(std::string& string, char substr, char replace) noexcept;

// Replaces a regex pattern with another string within a string
LOVU_FORCEINLINE LOVU_DLL void re_replace(std::string& string, const std::regex& pattern, const std::string& replace) noexcept;

// Replaces any occurence of a \ with a /
LOVU_FORCEINLINE LOVU_DLL void clean_os_path(std::string& string) noexcept;

// Replaces any occurence of a \ with a /
LOVU_FORCEINLINE LOVU_DLL std::string clean_os_path(const std::string& string) noexcept;

// Formats a string with arguments
template<typename... Args>
LOVU_FORCEINLINE LOVU_DLL void format(const char* fmt, Args&&... args) noexcept;

// Splits a string and returns a vector containing each string separated by the specified delimiter
LOVU_FORCEINLINE LOVU_DLL void split(std::vector<std::string>& output_strings, const std::string& input_string, char delimiter) noexcept;

STR_NAMESPACE_END

LOVU_NAMESPACE_END