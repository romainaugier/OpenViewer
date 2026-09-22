// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.
//
// Minimal test harness shared by every libOpenViewer test.
//
// Deliberately small: no dependency, no build system integration beyond one
// executable per file, and a failure prints the file, the line and both values.
// If this ever needs fixtures or parameterised cases, replace it with doctest
// rather than growing it.
//
// Usage:
//
//     #include "lov_test.hpp"
//
//     LOV_TEST(my_case)
//     {
//         LOV_CHECK_EQ(2 + 2, 4);
//     }
//
//     LOV_TEST_MAIN()

#pragma once

#if !defined(__LOV_TEST)
#define __LOV_TEST

#include "OpenViewer/log.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace lov_test
{

struct TestCase
{
    const char* name;
    void (*func)();
};

inline std::vector<TestCase>& registry() noexcept
{
    static std::vector<TestCase> cases;
    return cases;
}

inline int& failure_count() noexcept
{
    static int count = 0;
    return count;
}

struct Registrar
{
    Registrar(const char* name, void (*func)())
    {
        registry().push_back(TestCase{name, func});
    }
};

// Thrown by LOV_REQUIRE to abandon the current case while letting the rest run.
struct Abort
{
};

inline void report_failure(const char* file, int line, const std::string& message) noexcept
{
    std::fprintf(stderr, "  FAILED %s:%d\n    %s\n", file, line, message.c_str());
    ++failure_count();
}

template <typename T>
inline std::string to_display(const T& value)
{
    if constexpr(std::is_convertible_v<T, std::string>)
    {
        return std::string(value);
    }
    else if constexpr(std::is_floating_point_v<T> || std::is_integral_v<T>)
    {
        return std::to_string(value);
    }
    else
    {
        return "<value>";
    }
}

// Unique temporary directory for one test binary, removed on exit.
class ScratchDir
{
private:
    std::filesystem::path _path;

public:
    explicit ScratchDir(const char* tag)
    {
        this->_path = std::filesystem::temp_directory_path() /
                      (std::string("openviewer_tests_") + tag + "_" +
                       std::to_string(static_cast<unsigned long long>(
                           std::hash<std::string>{}(std::string(tag)))));

        std::filesystem::remove_all(this->_path);
        std::filesystem::create_directories(this->_path);
    }

    ~ScratchDir()
    {
        std::error_code ec;
        std::filesystem::remove_all(this->_path, ec);
    }

    std::string file(const char* name) const
    {
        return (this->_path / name).string();
    }
};

// Flushes every logger and returns the whole log file, for tests that check what
// was written
inline std::string read_log_file()
{
    lov::log::flush();

    std::ifstream file(lov::log::file_path().c_str(), std::ios::binary);

    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

// True if one line of the log holds both strings
inline bool log_has_line(const std::string& log, const std::string& a, const std::string& b)
{
    std::size_t start = 0;

    while(start < log.size())
    {
        std::size_t end = log.find('\n', start);

        if(end == std::string::npos)
            end = log.size();

        const std::string line = log.substr(start, end - start);

        if(line.find(a) != std::string::npos && line.find(b) != std::string::npos)
            return true;

        start = end + 1;
    }

    return false;
}

// LOV_TEST_FILTER=<substring> runs only the cases whose name contains it,
// e.g. LOV_TEST_FILTER=ring ./test_media_cache
inline int run_all(const char* binary_name) noexcept
{
    const char* filter = std::getenv("LOV_TEST_FILTER");

    std::fprintf(stderr, "== %s: %zu case(s) ==\n", binary_name, registry().size());

    for(const auto& test_case : registry())
    {
        if(filter != nullptr && std::strstr(test_case.name, filter) == nullptr)
        {
            continue;
        }

        const int before = failure_count();

        std::fprintf(stderr, "-- %s\n", test_case.name);

        try
        {
            test_case.func();
        }
        catch(const Abort&)
        {
            // Already reported by LOV_REQUIRE.
        }
        catch(const std::exception& e)
        {
            report_failure(__FILE__, __LINE__, std::string("unexpected exception: ") + e.what());
        }

        if(failure_count() == before)
        {
            std::fprintf(stderr, "   ok\n");
        }
    }

    if(failure_count() > 0)
    {
        std::fprintf(stderr, "== %s: %d failure(s) ==\n", binary_name, failure_count());
        return 1;
    }

    std::fprintf(stderr, "== %s: all passed ==\n", binary_name);

    return 0;
}

} // namespace lov_test

#define LOV_TEST(name)                                                                             \
    static void name();                                                                            \
    static lov_test::Registrar CONCAT(registrar_, name)(#name, name);                              \
    static void name()

#define LOV_CHECK(expr)                                                                            \
    do                                                                                             \
    {                                                                                              \
        if(!(expr))                                                                                \
        {                                                                                          \
            lov_test::report_failure(__FILE__, __LINE__, "expected: " #expr);                      \
        }                                                                                          \
    } while(0)

#define LOV_REQUIRE(expr)                                                                          \
    do                                                                                             \
    {                                                                                              \
        if(!(expr))                                                                                \
        {                                                                                          \
            lov_test::report_failure(__FILE__, __LINE__, "required: " #expr);                      \
            throw lov_test::Abort{};                                                               \
        }                                                                                          \
    } while(0)

#define LOV_CHECK_EQ(lhs, rhs)                                                                     \
    do                                                                                             \
    {                                                                                              \
        const auto lov_lhs = (lhs);                                                                \
        const auto lov_rhs = (rhs);                                                                \
        if(!(lov_lhs == lov_rhs))                                                                  \
        {                                                                                          \
            lov_test::report_failure(__FILE__,                                                     \
                                     __LINE__,                                                     \
                                     std::string(#lhs " == " #rhs " (") +                          \
                                         lov_test::to_display(lov_lhs) + " vs " +                  \
                                         lov_test::to_display(lov_rhs) + ")");                     \
        }                                                                                          \
    } while(0)

#define LOV_CHECK_NEAR(lhs, rhs, epsilon)                                                          \
    do                                                                                             \
    {                                                                                              \
        const double lov_lhs = static_cast<double>(lhs);                                           \
        const double lov_rhs = static_cast<double>(rhs);                                           \
        if(std::fabs(lov_lhs - lov_rhs) > static_cast<double>(epsilon))                            \
        {                                                                                          \
            lov_test::report_failure(__FILE__,                                                     \
                                     __LINE__,                                                     \
                                     std::string(#lhs " ~= " #rhs " (") +                          \
                                         std::to_string(lov_lhs) + " vs " +                        \
                                         std::to_string(lov_rhs) + ")");                           \
        }                                                                                          \
    } while(0)

#define LOV_TEST_MAIN()                                                                            \
    int main(int argc, char** argv)                                                                \
    {                                                                                              \
        LOV_UNUSED(argc);                                                                          \
        lov::log::initialize(spdlog::level::warn);                                                 \
        return lov_test::run_all(argv[0]);                                                         \
    }

#endif // !defined(__LOV_TEST)
