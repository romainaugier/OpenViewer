#include "string_utils.h"

bool endsWith(std::string_view str, std::string_view suffix) constexpr
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

bool startsWith(std::string_view str, std::string_view prefix) constexpr
{
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}