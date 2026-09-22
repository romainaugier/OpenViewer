// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "lov_test.hpp"
#include "test_images.hpp"

#include "OpenViewer/media.hpp"

#include <vector>

using namespace lov;

LOV_TEST(sequence_patterns)
{
    LOV_CHECK_EQ(format_sequence_path(stdromano::StringD::make_ref("render.####.exr"), 1001),
                 stdromano::StringD::make_ref("render.1001.exr"));

    LOV_CHECK_EQ(format_sequence_path(stdromano::StringD::make_ref("render.####.exr"), 7),
                 stdromano::StringD::make_ref("render.0007.exr"));

    LOV_CHECK_EQ(format_sequence_path(stdromano::StringD::make_ref("render.%04d.exr"), 42),
                 stdromano::StringD::make_ref("render.0042.exr"));

    // Padding too small for the frame number: the number wins
    LOV_CHECK_EQ(format_sequence_path(stdromano::StringD::make_ref("r.##.exr"), 1001),
                 stdromano::StringD::make_ref("r.1001.exr"));

    // No pattern at all: unchanged
    LOV_CHECK_EQ(format_sequence_path(stdromano::StringD::make_ref("still.exr"), 12),
                 stdromano::StringD::make_ref("still.exr"));
}

LOV_TEST(image_media_open_and_read)
{
    lov_test::ScratchDir dir("media");

    const std::string path = dir.file("still.exr");
    lov_test::write_exr(path, 24, 12, {"R", "G", "B", "A"}, Imf::HALF);

    ImageMedia media(stdromano::StringD::make_from_c_str(path.c_str()));

    LOV_REQUIRE(media.open());
    LOV_CHECK_EQ(media.length(), std::uint32_t(1));
    LOV_CHECK(media.contains_frame(0));
    LOV_CHECK(!media.contains_frame(1));

    const stdromano::StringD main_layer = stdromano::StringD::make_ref(MediaInfo::MAIN_LAYER_NAME);

    const std::size_t frame_size = media.frame_size(main_layer);
    LOV_CHECK_EQ(frame_size, std::size_t(24) * 12 * 4 * 2);

    std::vector<char> buffer(frame_size, 0);

    LOV_CHECK(media.read_frame(0, main_layer, buffer.data(), buffer.size()));

    // Out of range and unknown layer both fail rather than reading something.
    LOV_CHECK(!media.read_frame(1, main_layer, buffer.data(), buffer.size()));
    LOV_CHECK(!media.read_frame(0,
                                stdromano::StringD::make_ref("nope"),
                                buffer.data(),
                                buffer.size()));
}

LOV_TEST(image_sequence_media)
{
    lov_test::ScratchDir dir("sequence");

    for(std::uint32_t frame = 1001; frame <= 1004; ++frame)
    {
        const std::string name = "seq." + std::to_string(frame) + ".exr";
        lov_test::write_exr(dir.file(name.c_str()), 16, 16, {"R", "G", "B"}, Imf::HALF);
    }

    const std::string pattern = dir.file("seq.####.exr");

    ImageSequenceMedia media(stdromano::StringD::make_from_c_str(pattern.c_str()), 1001, 1004);

    LOV_REQUIRE(media.open());
    LOV_CHECK_EQ(media.length(), std::uint32_t(4));

    const stdromano::StringD main_layer =
        stdromano::StringD::make_ref(MediaInfo::MAIN_LAYER_NAME);

    std::vector<char> buffer(media.frame_size(main_layer), 0);

    for(std::uint32_t frame = 1001; frame <= 1004; ++frame)
    {
        LOV_CHECK(media.read_frame(frame, main_layer, buffer.data(), buffer.size()));
    }

    LOV_CHECK(!media.read_frame(1005, main_layer, buffer.data(), buffer.size()));
    LOV_CHECK_EQ(media.frame_path(1005).size(), std::size_t(0));
}

LOV_TEST_MAIN()
