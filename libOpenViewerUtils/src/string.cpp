// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Romain Augier
// All rights reserved.

#include "OpenViewerUtils/string.h"

#include <sstream>

LOVU_NAMESPACE_BEGIN

STR_NAMESPACE_BEGIN

LOVU_FORCEINLINE LOVU_DLL bool ends_with(const std::string& str, const std::string& suffix) noexcept
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

LOVU_FORCEINLINE LOVU_DLL bool ends_with(const std::string_view& str, const std::string& suffix) noexcept
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

LOVU_FORCEINLINE LOVU_DLL bool starts_with(const std::string& str, const std::string& prefix) noexcept
{
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

LOVU_FORCEINLINE LOVU_DLL bool starts_with(const std::string_view& str, const std::string& prefix) noexcept
{
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

LOVU_DLL void strip(std::string& s) noexcept
{
    char space = ' ';

    if ((char)s[0] == space) s.erase(0, 1);
    if ((char)s[s.size() - 1] == space) s.erase(s.size() - 1);
}

LOVU_FORCEINLINE LOVU_DLL void replace(std::string& string, char substr, char replace) noexcept
{
    std::replace(string.begin(), string.end(), substr, replace);
}

LOVU_FORCEINLINE LOVU_DLL void re_replace(std::string& string, const std::regex& pattern, const std::string& replace) noexcept
{
    string = std::regex_replace(string, pattern, replace);
}

LOVU_FORCEINLINE LOVU_DLL void clean_os_path(std::string& string) noexcept
{
    std::replace(string.begin(), string.end(), '\\', '/');
}

LOVU_FORCEINLINE LOVU_DLL std::string clean_os_path(const std::string& string) noexcept
{
    std::string new_string = string;
    clean_os_path(new_string);
    return new_string;
}

template<typename... Args>
LOVU_FORCEINLINE LOVU_DLL void format(const char* fmt, Args&&... args) noexcept
{
    return std::string msg = fmt::format(fmt, fmt::make_args_checked(fmt, args...)); 
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