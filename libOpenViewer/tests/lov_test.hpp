// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.
//
// Fixtures shared by every libOpenViewer test, on top of the stdromano harness
// (stdromano/test.hpp) and fuzzer (stdromano/fuzz.hpp).
//
//     #include "lov_test.hpp"
//
//     STDROMANO_TEST_CASE(my_case)
//     {
//         STDROMANO_CHECK_EQ(2 + 2, 4);
//     }
//
//     LOV_TEST_MAIN()
//
// ./test_media_cache ring     runs the cases whose name contains "ring"
// ./test_media_cache --list   lists the cases
// STDROMANO_TEST_FILTER=ring  same filter, for ctest
// ROMANO_FUZZ_SCALE=100       runs the fuzz cases 100 times longer

#pragma once

#if !defined(__LOV_TEST)
#define __LOV_TEST

#include "OpenViewer/log.hpp"

#include "stdromano/fuzz.hpp"
#include "stdromano/test.hpp"

#include <array>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace lov_test
{

inline std::string temp_path(const char* name)
{
    const stdromano::StringD path = stdromano::test::temp_path(name);
    return std::string(path.c_str(), path.size());
}

inline stdromano::fuzz::Options fuzz_options(const char* name, std::uint64_t iterations)
{
    stdromano::fuzz::Options options;
    options.name = name;
    options.iterations = iterations;

#if !defined(NDEBUG)
    options.iterations = iterations / 4 + 1;
#endif // !defined(NDEBUG)

    return options;
}

// Fuzzed inputs are mostly invalid, and every one of them is logged by the code
// under test
class QuietLogs
{
    static constexpr std::size_t NUM_CATEGORIES = static_cast<std::size_t>(lov::LogCategory::Count);

    std::array<spdlog::level::level_enum, NUM_CATEGORIES> _previous;

public:
    QuietLogs()
    {
        for(std::size_t i = 0; i < NUM_CATEGORIES; ++i)
        {
            const lov::LogCategory category = static_cast<lov::LogCategory>(i);
            this->_previous[i] = lov::log::get(category).level();
            lov::log::set_level(category, spdlog::level::off);
        }
    }

    ~QuietLogs()
    {
        for(std::size_t i = 0; i < NUM_CATEGORIES; ++i)
            lov::log::set_level(static_cast<lov::LogCategory>(i), this->_previous[i]);
    }

    QuietLogs(const QuietLogs&) = delete;
    QuietLogs& operator=(const QuietLogs&) = delete;
};

inline std::vector<std::uint8_t> read_file_bytes(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);

    return std::vector<std::uint8_t>(std::istreambuf_iterator<char>(file),
                                     std::istreambuf_iterator<char>());
}

inline bool write_file_bytes(const std::string& path, const std::uint8_t* data, std::size_t size)
{
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    file.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(size));

    return static_cast<bool>(file);
}

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

} // namespace lov_test

#define LOV_REQUIRE_PROPERTY(report) STDROMANO_REQUIRE_MSG((report).passed(), (report).describe())

#define LOV_TEST_MAIN()                                                                            \
    int main(int argc, char** argv)                                                                \
    {                                                                                              \
        lov::log::initialize(spdlog::level::warn);                                                 \
        return stdromano::test::default_runner().run(argc, argv);                                  \
    }

#endif // !defined(__LOV_TEST)
