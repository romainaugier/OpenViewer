// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "lov_test.hpp"
#include "test_images.hpp"

#include "OpenViewer/image_reader.hpp"

#include <vector>

using namespace lov;

static lov_test::ScratchDir& scratch() noexcept
{
    static lov_test::ScratchDir dir("exr");
    return dir;
}

// Reads one channel of one pixel, in absolute (data-window) coordinates.
template <typename T>
static float sample(const std::vector<char>& buffer,
                    const MediaLayer& layer,
                    const Imath::Box2i& data_window,
                    int x,
                    int y,
                    int channel)
{
    const std::size_t local_x = static_cast<std::size_t>(x - data_window.min.x);
    const std::size_t local_y = static_cast<std::size_t>(y - data_window.min.y);

    const std::size_t offset = local_y * layer.y_stride() + local_x * layer.x_stride() +
                               static_cast<std::size_t>(channel) * layer.channel_size();

    T value;
    std::memcpy(&value, buffer.data() + offset, sizeof(T));

    return static_cast<float>(value);
}

LOV_TEST(read_info_rgba_half)
{
    const std::string path = scratch().file("rgba_half.exr");

    lov_test::write_exr(path, 64, 32, {"R", "G", "B", "A"}, Imf::HALF);

    MediaInfo info;
    LOV_REQUIRE(image_read_info(stdromano::StringD::make_from_c_str(path.c_str()), info));

    LOV_CHECK_EQ(info.data_width(), std::uint32_t(64));
    LOV_CHECK_EQ(info.data_height(), std::uint32_t(32));
    LOV_CHECK(!info.has_overscan());
    LOV_CHECK_EQ(info.nlayers(), std::size_t(1));

    const MediaLayer* layer = info.main();
    LOV_REQUIRE(layer != nullptr);

    LOV_CHECK_EQ(static_cast<int>(layer->format()), static_cast<int>(MediaFormat_RGBA));
    LOV_CHECK_EQ(static_cast<int>(layer->depth()), static_cast<int>(MediaDepth_F16));
    LOV_CHECK_EQ(layer->nbytes(), std::size_t(64) * 32 * 4 * 2);
}

// Channels come out in canonical order whatever order the file stores them in.
// The previous implementation sorted by descending first character, which gives
// RGBA by accident and reverses XYZ.
LOV_TEST(channels_are_ordered_canonically)
{
    const std::string path = scratch().file("shuffled.exr");

    lov_test::write_exr(path, 8, 8, {"B", "A", "R", "G"}, Imf::HALF);

    MediaInfo info;
    LOV_REQUIRE(image_read_info(stdromano::StringD::make_from_c_str(path.c_str()), info));

    const MediaLayer* layer = info.main();
    LOV_REQUIRE(layer != nullptr);
    LOV_REQUIRE(layer->channels().size() == 4);

    LOV_CHECK_EQ(std::string(layer->channels()[0].c_str()), std::string("R"));
    LOV_CHECK_EQ(std::string(layer->channels()[1].c_str()), std::string("G"));
    LOV_CHECK_EQ(std::string(layer->channels()[2].c_str()), std::string("B"));
    LOV_CHECK_EQ(std::string(layer->channels()[3].c_str()), std::string("A"));
}

LOV_TEST(xyz_layers_are_not_reversed)
{
    const std::string path = scratch().file("normals.exr");

    lov_test::write_exr(path, 8, 8, {"R", "G", "B", "N.X", "N.Y", "N.Z"}, Imf::FLOAT);

    MediaInfo info;
    LOV_REQUIRE(image_read_info(stdromano::StringD::make_from_c_str(path.c_str()), info));

    const MediaLayer* normals = info.find_layer(stdromano::StringD::make_ref("N"));
    LOV_REQUIRE(normals != nullptr);
    LOV_REQUIRE(normals->channels().size() == 3);

    LOV_CHECK_EQ(std::string(normals->channels()[0].c_str()), std::string("N.X"));
    LOV_CHECK_EQ(std::string(normals->channels()[1].c_str()), std::string("N.Y"));
    LOV_CHECK_EQ(std::string(normals->channels()[2].c_str()), std::string("N.Z"));
}

LOV_TEST(read_pixels_half_is_exact)
{
    const std::string path = scratch().file("pixels_half.exr");
    const stdromano::StringD spath = stdromano::StringD::make_from_c_str(path.c_str());

    lov_test::write_exr(path, 37, 19, {"R", "G", "B", "A"}, Imf::HALF);

    MediaInfo info;
    LOV_REQUIRE(image_read_info(spath, info));

    const MediaLayer* layer = info.main();
    LOV_REQUIRE(layer != nullptr);

    std::vector<char> buffer(layer->nbytes(), 0);

    LOV_REQUIRE(image_read_layer(spath,
                                 stdromano::StringD::make_ref(MediaInfo::MAIN_LAYER_NAME),
                                 *layer,
                                 buffer.data(),
                                 buffer.size()));

    for(int y = 0; y < 19; ++y)
    {
        for(int x = 0; x < 37; ++x)
        {
            for(int c = 0; c < 4; ++c)
            {
                LOV_CHECK_EQ(sample<half>(buffer, *layer, info.data_window(), x, y, c),
                             lov_test::expected_pixel(x, y, c));
            }
        }
    }
}

// The bug that would have shipped: with a data window that does not start at
// the origin, the Imf slice base has to be shifted back by the window origin.
// Without the shift the reader writes outside the buffer and the values land in
// the wrong pixels.
LOV_TEST(read_pixels_with_overscan)
{
    const std::string path = scratch().file("overscan.exr");
    const stdromano::StringD spath = stdromano::StringD::make_from_c_str(path.c_str());

    const Imath::Box2i data_window(Imath::V2i(-7, -5), Imath::V2i(40, 26));
    const Imath::Box2i display_window(Imath::V2i(0, 0), Imath::V2i(31, 17));

    lov_test::write_exr(path, data_window, display_window, {"R", "G", "B"}, Imf::FLOAT);

    MediaInfo info;
    LOV_REQUIRE(image_read_info(spath, info));

    LOV_CHECK(info.has_overscan());
    LOV_CHECK_EQ(info.data_width(), std::uint32_t(48));
    LOV_CHECK_EQ(info.data_height(), std::uint32_t(32));
    LOV_CHECK_EQ(info.display_width(), std::uint32_t(32));
    LOV_CHECK_EQ(info.display_height(), std::uint32_t(18));

    const MediaLayer* layer = info.main();
    LOV_REQUIRE(layer != nullptr);
    LOV_CHECK_EQ(layer->nbytes(), std::size_t(48) * 32 * 3 * 4);

    // Guard bytes on both sides: if the reader addresses outside the buffer,
    // this catches it even in a build without sanitizers.
    const std::size_t guard = 4096;
    std::vector<char> buffer(layer->nbytes() + 2 * guard, char(0xAB));

    LOV_REQUIRE(image_read_layer(spath,
                                 stdromano::StringD::make_ref(MediaInfo::MAIN_LAYER_NAME),
                                 *layer,
                                 buffer.data() + guard,
                                 layer->nbytes()));

    for(std::size_t i = 0; i < guard; ++i)
    {
        LOV_REQUIRE(buffer[i] == char(0xAB));
        LOV_REQUIRE(buffer[buffer.size() - 1 - i] == char(0xAB));
    }

    std::vector<char> pixels(buffer.begin() + guard, buffer.end() - guard);

    for(int y = data_window.min.y; y <= data_window.max.y; ++y)
    {
        for(int x = data_window.min.x; x <= data_window.max.x; ++x)
        {
            for(int c = 0; c < 3; ++c)
            {
                LOV_CHECK_EQ(sample<float>(pixels, *layer, data_window, x, y, c),
                             lov_test::expected_pixel(x, y, c));
            }
        }
    }
}

LOV_TEST(multilayer)
{
    const std::string path = scratch().file("multilayer.exr");
    const stdromano::StringD spath = stdromano::StringD::make_from_c_str(path.c_str());

    lov_test::write_exr(path,
                        16,
                        16,
                        {"R", "G", "B", "A", "diffuse.R", "diffuse.G", "diffuse.B"},
                        Imf::HALF);

    MediaInfo info;
    LOV_REQUIRE(image_read_info(spath, info));

    LOV_CHECK_EQ(info.nlayers(), std::size_t(2));

    const MediaLayer* diffuse = info.find_layer(stdromano::StringD::make_ref("diffuse"));
    LOV_REQUIRE(diffuse != nullptr);
    LOV_CHECK_EQ(static_cast<int>(diffuse->format()), static_cast<int>(MediaFormat_RGB));

    std::vector<char> buffer(diffuse->nbytes(), 0);

    LOV_REQUIRE(image_read_layer(spath,
                                 stdromano::StringD::make_ref("diffuse"),
                                 *diffuse,
                                 buffer.data(),
                                 buffer.size()));

    // Channel index within the layer, not within the file: diffuse.R is
    // channel 0 of the layer but channel 4 of the file. Reading the file index
    // is the mistake the full channel names exist to prevent.
    for(int c = 0; c < 3; ++c)
    {
        LOV_CHECK_EQ(sample<half>(buffer, *diffuse, info.data_window(), 3, 5, c),
                     lov_test::expected_pixel(3, 5, c + 4));
    }
}

// A frame missing a channel the rest of the sequence has must not fail the
// read: Imf fills it, alpha with 1.0 and everything else with 0.
LOV_TEST(missing_channels_are_filled)
{
    const std::string path = scratch().file("rgb_only.exr");
    const stdromano::StringD spath = stdromano::StringD::make_from_c_str(path.c_str());

    lov_test::write_exr(path, 8, 8, {"R", "G", "B"}, Imf::HALF);

    MediaLayer layer(8, 8, MediaFormat_RGBA, MediaDepth_F16);
    layer.add_channel(stdromano::StringD::make_from_c_str("R"));
    layer.add_channel(stdromano::StringD::make_from_c_str("G"));
    layer.add_channel(stdromano::StringD::make_from_c_str("B"));
    layer.add_channel(stdromano::StringD::make_from_c_str("A"));

    std::vector<char> buffer(layer.nbytes(), 0);

    const Imath::Box2i data_window(Imath::V2i(0, 0), Imath::V2i(7, 7));

    LOV_REQUIRE(image_read_layer(spath,
                                 stdromano::StringD::make_ref(MediaInfo::MAIN_LAYER_NAME),
                                 layer,
                                 buffer.data(),
                                 buffer.size()));

    LOV_CHECK_EQ(sample<half>(buffer, layer, data_window, 2, 2, 0),
                 lov_test::expected_pixel(2, 2, 0));
    LOV_CHECK_EQ(sample<half>(buffer, layer, data_window, 2, 2, 3), 1.0f);
}

LOV_TEST(undersized_destination_is_refused)
{
    const std::string path = scratch().file("small.exr");
    const stdromano::StringD spath = stdromano::StringD::make_from_c_str(path.c_str());

    lov_test::write_exr(path, 16, 16, {"R", "G", "B", "A"}, Imf::HALF);

    MediaInfo info;
    LOV_REQUIRE(image_read_info(spath, info));

    const MediaLayer* layer = info.main();
    LOV_REQUIRE(layer != nullptr);

    std::vector<char> buffer(layer->nbytes(), 0);

    LOV_CHECK(!image_read_layer(spath,
                                stdromano::StringD::make_ref(MediaInfo::MAIN_LAYER_NAME),
                                *layer,
                                buffer.data(),
                                buffer.size() - 1));

    LOV_CHECK(!image_read_layer(spath,
                                stdromano::StringD::make_ref(MediaInfo::MAIN_LAYER_NAME),
                                *layer,
                                nullptr,
                                buffer.size()));
}

// A sequence where one frame was rendered at a different resolution must be
// caught, not read into a buffer sized for another frame.
LOV_TEST(resolution_mismatch_is_refused)
{
    const std::string small_path = scratch().file("res_small.exr");
    const std::string big_path = scratch().file("res_big.exr");

    lov_test::write_exr(small_path, 16, 16, {"R", "G", "B", "A"}, Imf::HALF);
    lov_test::write_exr(big_path, 32, 32, {"R", "G", "B", "A"}, Imf::HALF);

    MediaInfo info;
    LOV_REQUIRE(image_read_info(stdromano::StringD::make_from_c_str(small_path.c_str()), info));

    const MediaLayer* layer = info.main();
    LOV_REQUIRE(layer != nullptr);

    std::vector<char> buffer(layer->nbytes(), 0);

    LOV_CHECK(!image_read_layer(stdromano::StringD::make_from_c_str(big_path.c_str()),
                                stdromano::StringD::make_ref(MediaInfo::MAIN_LAYER_NAME),
                                *layer,
                                buffer.data(),
                                buffer.size()));
}

LOV_TEST_MAIN()
