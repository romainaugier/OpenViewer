// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "fuzz_targets.hpp"
#include "lov_test.hpp"
#include "test_images.hpp"

#include "OpenViewer/media.hpp"

#include <vector>

using namespace lov;

STDROMANO_TEST_CASE(sequence_patterns)
{
    STDROMANO_CHECK_EQ(format_sequence_path(stdromano::StringD::make_ref("render.####.exr"), 1001),
                 stdromano::StringD::make_ref("render.1001.exr"));

    STDROMANO_CHECK_EQ(format_sequence_path(stdromano::StringD::make_ref("render.####.exr"), 7),
                 stdromano::StringD::make_ref("render.0007.exr"));

    STDROMANO_CHECK_EQ(format_sequence_path(stdromano::StringD::make_ref("render.%04d.exr"), 42),
                 stdromano::StringD::make_ref("render.0042.exr"));

    // Padding too small for the frame number: the number wins
    STDROMANO_CHECK_EQ(format_sequence_path(stdromano::StringD::make_ref("r.##.exr"), 1001),
                 stdromano::StringD::make_ref("r.1001.exr"));

    STDROMANO_CHECK_EQ(format_sequence_path(stdromano::StringD::make_ref("render.%d.exr"), 42),
                       stdromano::StringD::make_ref("render.42.exr"));

    // Frames are unsigned: past INT_MAX they must not come out negative
    STDROMANO_CHECK_EQ(format_sequence_path(stdromano::StringD::make_ref("r.%04d.exr"), 4000000000u),
                       stdromano::StringD::make_ref("r.4000000000.exr"));

    // Only the frame token is expanded, anything else in a path stays as is
    STDROMANO_CHECK_EQ(format_sequence_path(stdromano::StringD::make_ref("100%s/r.%n.exr"), 7),
                       stdromano::StringD::make_ref("100%s/r.%n.exr"));

    // No pattern at all: unchanged
    STDROMANO_CHECK_EQ(format_sequence_path(stdromano::StringD::make_ref("still.exr"), 12),
                 stdromano::StringD::make_ref("still.exr"));
}

STDROMANO_TEST_CASE(image_media_open_and_read)
{
    const std::string path = lov_test::temp_path("still.exr");
    lov_test::write_exr(path, 24, 12, {"R", "G", "B", "A"}, Imf::HALF);

    ImageMedia media(stdromano::StringD::make_from_c_str(path.c_str()));

    STDROMANO_REQUIRE(media.open());
    STDROMANO_CHECK_EQ(media.length(), std::uint32_t(1));
    STDROMANO_CHECK(media.contains_frame(0));
    STDROMANO_CHECK(!media.contains_frame(1));

    const stdromano::StringD main_layer = stdromano::StringD::make_ref(MediaInfo::MAIN_LAYER_NAME);

    const std::size_t frame_size = media.frame_size(main_layer);
    STDROMANO_CHECK_EQ(frame_size, std::size_t(24) * 12 * 4 * 2);

    std::vector<char> buffer(frame_size, 0);

    STDROMANO_CHECK(media.read_frame(0, main_layer, buffer.data(), buffer.size()));

    // Out of range and unknown layer both fail rather than reading something.
    STDROMANO_CHECK(!media.read_frame(1, main_layer, buffer.data(), buffer.size()));
    STDROMANO_CHECK(!media.read_frame(0,
                                stdromano::StringD::make_ref("nope"),
                                buffer.data(),
                                buffer.size()));
}

STDROMANO_TEST_CASE(image_sequence_media)
{
    for(std::uint32_t frame = 1001; frame <= 1004; ++frame)
    {
        const std::string name = "seq." + std::to_string(frame) + ".exr";
        lov_test::write_exr(lov_test::temp_path(name.c_str()), 16, 16, {"R", "G", "B"}, Imf::HALF);
    }

    const std::string pattern = lov_test::temp_path("seq.####.exr");

    ImageSequenceMedia media(stdromano::StringD::make_from_c_str(pattern.c_str()), 1001, 1004);

    STDROMANO_REQUIRE(media.open());
    STDROMANO_CHECK_EQ(media.length(), std::uint32_t(4));

    const stdromano::StringD main_layer =
        stdromano::StringD::make_ref(MediaInfo::MAIN_LAYER_NAME);

    std::vector<char> buffer(media.frame_size(main_layer), 0);

    for(std::uint32_t frame = 1001; frame <= 1004; ++frame)
    {
        STDROMANO_CHECK(media.read_frame(frame, main_layer, buffer.data(), buffer.size()));
    }

    STDROMANO_CHECK(!media.read_frame(1005, main_layer, buffer.data(), buffer.size()));
    STDROMANO_CHECK_EQ(media.frame_path(1005).size(), std::size_t(0));
}

STDROMANO_TEST_CASE(fuzz_sequence_patterns_expand)
{
    const auto report = stdromano::fuzz::run_property(lov_test::fuzz_options("sequence_patterns_expand", 2000),
                                                      lov_fuzz::sequence_pattern_expands);

    LOV_REQUIRE_PROPERTY(report);
}

STDROMANO_TEST_CASE(fuzz_sequence_patterns_from_any_path)
{
    auto options = lov_test::fuzz_options("sequence_patterns_from_any_path", 3000);
    options.max_input_size = 512;
    options.dictionary.tokens = {"%", "%d", "%04d", "%s", "%n", "%x", "%p", "%%", "%*d", "%099999d", "####", "#"};
    options.corpus = {{0xE9, 0x03, 0, 0, 'r', '.', '%', '0', '4', 'd', '.', 'e', 'x', 'r'},
                      {0x01, 0, 0, 0, 's', 'h', '.', '#', '#', '#', '#', '.', 'e', 'x', 'r'}};

    lov_test::QuietLogs quiet;

    const auto report = stdromano::fuzz::run_input(options, lov_fuzz::sequence_pattern_any);

    LOV_REQUIRE_PROPERTY(report);
}

LOV_TEST_MAIN()
