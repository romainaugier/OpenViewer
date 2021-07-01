// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#include "string_utils.h"

bool endsWith(std::string_view str, std::string_view suffix)
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

bool startsWith(std::string_view str, std::string_view prefix)
{
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

const char* remove_spaces(char* s)
{
    std::string tmp = s;

    char space[2] = " ";

    if ((char*)tmp[0] == space) tmp.erase(0, 1);
    if ((char*)tmp[tmp.size() - 1] == space) tmp.erase(tmp.size() - 1);

    return tmp.c_str();

    /*
    while (*cpy)
    {
        if (*cpy != ' ')
            *temp++ = *cpy;
        cpy++;
    }
    *temp = 0;
    */
}
