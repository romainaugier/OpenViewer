// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.
//
// Fuzz targets shared by the test cases (bounded, seeded runs under ctest) and
// the libFuzzer executables in fuzz/ (open ended runs, OPENVIEWER_BUILD_FUZZERS)

#pragma once

#if !defined(__LOV_FUZZ_TARGETS)
#define __LOV_FUZZ_TARGETS

#include "lov_test.hpp"

#include "OpenViewer/image_reader.hpp"
#include "OpenViewer/log.hpp"
#include "OpenViewer/media.hpp"
#include "OpenViewer/media_cache.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstring>
#include <map>
#include <string>
#include <vector>

namespace lov_fuzz
{

using stdromano::fuzz::Source;

inline std::string zero_padded(std::uint32_t value, std::size_t width)
{
    std::string digits = std::to_string(value);

    if(digits.size() < width)
        digits.insert(0, width - digits.size(), '0');

    return digits;
}

inline std::string to_std(const stdromano::StringD& str)
{
    return std::string(str.c_str(), str.size());
}

inline stdromano::StringD to_stringd(const std::string& str)
{
    return stdromano::StringD::make_from_c_str(str.c_str(), str.size());
}

inline std::string draw_path_part(Source& source)
{
    const stdromano::StringD part = source.string(24, "abcXYZ019_-./:\\ ");
    return to_std(part);
}

/* ------------------------------------------------------------------------ */
/* Sequence patterns                                                        */
/* ------------------------------------------------------------------------ */

inline bool sequence_pattern_expands(Source& source)
{
    const std::string prefix = draw_path_part(source);
    const std::string suffix = draw_path_part(source);
    const std::uint32_t frame = source.integer<std::uint32_t>();
    const std::size_t padding = source.range<std::size_t>(1, 12);

    const std::string expected = prefix + zero_padded(frame, padding) + suffix;

    const std::string hashes = prefix + std::string(padding, '#') + suffix;
    STDROMANO_FUZZ_CHECK_EQ(to_std(lov::format_sequence_path(to_stringd(hashes), frame)), expected);

    const std::string printf_style = prefix + "%0" + std::to_string(padding) + "d" + suffix;
    STDROMANO_FUZZ_CHECK_EQ(to_std(lov::format_sequence_path(to_stringd(printf_style), frame)),
                            expected);

    const std::string still = prefix + suffix;
    STDROMANO_FUZZ_CHECK_EQ(to_std(lov::format_sequence_path(to_stringd(still), frame)), still);

    return true;
}

// A pattern is a path picked in a file dialog: anything can be in it, and none of
// it may be interpreted as more than a frame number
inline bool sequence_pattern_any(const std::uint8_t* data, std::size_t size)
{
    std::uint32_t frame = 0;

    const std::size_t frame_bytes = std::min(size, sizeof(frame));
    if(frame_bytes > 0)
        std::memcpy(&frame, data, frame_bytes);

    const std::string pattern(reinterpret_cast<const char*>(data) + frame_bytes, size - frame_bytes);

    const std::string result = to_std(lov::format_sequence_path(to_stringd(pattern), frame));

    if(pattern.find_first_of("#%") == std::string::npos)
        STDROMANO_FUZZ_CHECK_EQ(result, pattern);

    return true;
}

/* ------------------------------------------------------------------------ */
/* Log levels                                                               */
/* ------------------------------------------------------------------------ */

inline constexpr std::size_t NUM_CATEGORIES = static_cast<std::size_t>(lov::LogCategory::Count);

using Levels = std::array<spdlog::level::level_enum, NUM_CATEGORIES>;

inline Levels current_levels() noexcept
{
    Levels levels;

    for(std::size_t i = 0; i < NUM_CATEGORIES; ++i)
        levels[i] = lov::log::get(static_cast<lov::LogCategory>(i)).level();

    return levels;
}

inline void set_levels(const Levels& levels) noexcept
{
    for(std::size_t i = 0; i < NUM_CATEGORIES; ++i)
        lov::log::set_level(static_cast<lov::LogCategory>(i), levels[i]);
}

inline Levels draw_levels(Source& source)
{
    Levels levels;

    for(auto& level : levels)
        level = static_cast<spdlog::level::level_enum>(source.below(spdlog::level::n_levels));

    return levels;
}

inline std::string random_case(Source& source, const char* name)
{
    std::string result(name);

    for(char& c : result)
        if(source.boolean())
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

    return result;
}

inline std::string blanks(Source& source)
{
    return std::string(source.below(3), source.boolean() ? ' ' : '\t');
}

// Builds a valid spec from a model of what it should do, and checks that
// apply_levels does exactly that: last entry wins, untouched categories keep
// their level
inline bool apply_levels_matches_model(Source& source)
{
    static const std::pair<const char*, spdlog::level::level_enum> LEVEL_NAMES[] = {
        {"trace", spdlog::level::trace},
        {"debug", spdlog::level::debug},
        {"info", spdlog::level::info},
        {"warn", spdlog::level::warn},
        {"warning", spdlog::level::warn},
        {"error", spdlog::level::err},
        {"err", spdlog::level::err},
        {"critical", spdlog::level::critical},
        {"off", spdlog::level::off},
    };

    const Levels previous = current_levels();

    Levels expected = draw_levels(source);
    set_levels(expected);

    std::string spec;

    const std::size_t nentries = source.size(8);

    for(std::size_t i = 0; i < nentries; ++i)
    {
        if(i > 0 || source.one_in(8))
            spec += ',';

        if(source.one_in(8))
            continue;

        const auto& level = LEVEL_NAMES[source.index(std::size(LEVEL_NAMES))];

        spec += blanks(source);

        if(source.one_in(4))
        {
            spec += random_case(source, level.first);
            expected.fill(level.second);
        }
        else
        {
            const std::size_t category = source.index(NUM_CATEGORIES);

            spec += random_case(source, lov::log::category_name(static_cast<lov::LogCategory>(category)));
            spec += blanks(source) + "=" + blanks(source);
            spec += random_case(source, level.first);

            expected[category] = level.second;
        }

        spec += blanks(source);
    }

    const bool applied = lov::log::apply_levels(spec.c_str());
    const Levels actual = current_levels();

    set_levels(previous);

    if(!applied)
        throw stdromano::fuzz::PropertyFailure("valid spec refused: \"" + spec + "\"");

    for(std::size_t i = 0; i < NUM_CATEGORIES; ++i)
        if(actual[i] != expected[i])
            throw stdromano::fuzz::PropertyFailure("\"" + spec + "\" set " +
                                                   lov::log::category_name(static_cast<lov::LogCategory>(i)) +
                                                   " to " + std::to_string(actual[i]) + " instead of " +
                                                   std::to_string(expected[i]));

    return true;
}

// Whatever the spec, a refused one changes nothing
inline bool apply_levels_any(const std::uint8_t* data, std::size_t size)
{
    const Levels previous = current_levels();

    Source source = Source::from_seed(size > 0 ? data[0] : 0);
    const Levels before = draw_levels(source);
    set_levels(before);

    const std::string spec(reinterpret_cast<const char*>(data), size);

    const bool applied = lov::log::apply_levels(spec.c_str());
    const Levels after = current_levels();

    set_levels(previous);

    if(!applied)
        STDROMANO_FUZZ_CHECK(after == before);

    return true;
}

inline stdromano::fuzz::Dictionary log_spec_dictionary()
{
    stdromano::fuzz::Dictionary dictionary;

    dictionary.tokens = {"=", ",", " ", "\t", "trace", "debug", "info", "warn", "warning", "error",
                         "err", "critical", "off", "OFF"};

    for(std::size_t i = 0; i < NUM_CATEGORIES; ++i)
        dictionary.tokens.emplace_back(lov::log::category_name(static_cast<lov::LogCategory>(i)));

    return dictionary;
}

/* ------------------------------------------------------------------------ */
/* Media cache                                                              */
/* ------------------------------------------------------------------------ */

// Generous upper bound of the per-block header: a block that fits with this much
// room left must be served, however fragmented the ring is
inline constexpr std::size_t CACHE_HEADER_BUDGET = 256;

inline bool media_cache_keeps_live_blocks_intact(Source& source)
{
    struct Live
    {
        unsigned char* data;
        std::size_t size;
        unsigned char fill;
    };

    const std::size_t capacity = source.one_in(8) ? source.range<std::size_t>(0, 512)
                                                  : source.range<std::size_t>(4096, 256 * 1024);

    std::map<std::uint32_t, Live> live;
    std::uint32_t allocated = 0;
    std::uint32_t destroyed = 0;

    const auto check_live_blocks = [&]() {
        std::vector<const Live*> by_address;
        std::size_t live_bytes = 0;

        for(const auto& entry : live)
        {
            const Live& block = entry.second;

            STDROMANO_FUZZ_CHECK(block.data[0] == block.fill);
            STDROMANO_FUZZ_CHECK(block.data[block.size / 2] == block.fill);
            STDROMANO_FUZZ_CHECK(block.data[block.size - 1] == block.fill);

            live_bytes += block.size;
            by_address.push_back(&block);
        }

        std::sort(by_address.begin(), by_address.end(), [](const Live* a, const Live* b) {
            return a->data < b->data;
        });

        for(std::size_t i = 1; i < by_address.size(); ++i)
            STDROMANO_FUZZ_CHECK(by_address[i - 1]->data + by_address[i - 1]->size <= by_address[i]->data);

        STDROMANO_FUZZ_CHECK(live_bytes <= capacity);
    };

    {
        lov::MediaCache cache(capacity);

        const std::size_t nops = source.range<std::size_t>(1, 64);

        for(std::size_t op = 0; op < nops; ++op)
        {
            if(source.one_in(16))
            {
                cache.clear();
                STDROMANO_FUZZ_CHECK(live.empty());
                STDROMANO_FUZZ_CHECK_EQ(cache.get_used_bytes(), std::size_t(0));
                continue;
            }

            const std::size_t size = source.one_in(8) ? source.integer<std::size_t>()
                                                      : source.range<std::size_t>(0, capacity + capacity / 4);

            const std::uint32_t id = allocated;

            void* block = cache.allocate(size, [&live, &destroyed, id]() -> void {
                live.erase(id);
                ++destroyed;
            });

            if(block == nullptr)
            {
                STDROMANO_FUZZ_CHECK(size == 0 || size > capacity || capacity - size < CACHE_HEADER_BUDGET);
                continue;
            }

            STDROMANO_FUZZ_CHECK(size > 0);
            STDROMANO_FUZZ_CHECK(size <= capacity);

            ++allocated;

            const unsigned char fill = static_cast<unsigned char>(source.below(256));
            std::memset(block, fill, size);

            live[id] = Live{static_cast<unsigned char*>(block), size, fill};

            STDROMANO_FUZZ_CHECK(cache.get_used_bytes() <= capacity);
            check_live_blocks();
        }
    }

    STDROMANO_FUZZ_CHECK(live.empty());
    STDROMANO_FUZZ_CHECK_EQ(destroyed, allocated);

    return true;
}

/* ------------------------------------------------------------------------ */
/* Image readers                                                            */
/* ------------------------------------------------------------------------ */

inline constexpr std::size_t MAX_FUZZED_LAYER_BYTES = 64 * 1024 * 1024;

// Any file, however broken, is refused or read within the destination buffer
inline bool image_file_is_handled(const char* extension, const std::uint8_t* data, std::size_t size)
{
    static const std::string path_prefix = lov_test::temp_path("fuzz_input.");

    const std::string path = path_prefix + extension;

    if(!lov_test::write_file_bytes(path, data, size))
        throw stdromano::fuzz::PropertyFailure("cannot write " + path);

    const stdromano::StringD spath = to_stringd(path);

    lov::MediaInfo info;

    if(!lov::image_read_info(spath, info))
        return true;

    for(const auto& entry : info.layers())
    {
        const lov::MediaLayer& layer = entry.second;

        STDROMANO_FUZZ_CHECK_EQ(layer.width(), info.data_width());
        STDROMANO_FUZZ_CHECK_EQ(layer.height(), info.data_height());

        if(layer.nbytes() == 0 || layer.nbytes() > MAX_FUZZED_LAYER_BYTES)
            continue;

        constexpr std::size_t GUARD = 256;
        constexpr char GUARD_BYTE = char(0xAB);

        std::vector<char> buffer(layer.nbytes() + 2 * GUARD, GUARD_BYTE);

        lov::image_read_layer(spath, entry.first, layer, buffer.data() + GUARD, layer.nbytes());

        for(std::size_t i = 0; i < GUARD; ++i)
        {
            STDROMANO_FUZZ_CHECK(buffer[i] == GUARD_BYTE);
            STDROMANO_FUZZ_CHECK(buffer[buffer.size() - 1 - i] == GUARD_BYTE);
        }
    }

    return true;
}

} // namespace lov_fuzz

#endif // !defined(__LOV_FUZZ_TARGETS)
