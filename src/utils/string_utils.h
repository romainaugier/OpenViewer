#ifdef _MSC_VER
#include "stringview.h"
#elif __GNUC__
#include "string_view"
#endif

bool endsWith(std::string_view str, std::string_view suffix);

bool startsWith(std::string_view str, std::string_view prefix);