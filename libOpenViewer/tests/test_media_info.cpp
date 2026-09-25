// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "lov_test.hpp"

#include "OpenViewer/media_info.hpp"

using namespace lov;

STDROMANO_TEST_CASE(depth_sizes)
{
    STDROMANO_CHECK_EQ(depth_size(MediaDepth_U8), std::size_t(1));
    STDROMANO_CHECK_EQ(depth_size(MediaDepth_F16), std::size_t(2));
    STDROMANO_CHECK_EQ(depth_size(MediaDepth_U16), std::size_t(2));
    STDROMANO_CHECK_EQ(depth_size(MediaDepth_F32), std::size_t(4));
    STDROMANO_CHECK_EQ(depth_size(MediaDepth_U32), std::size_t(4));
    STDROMANO_CHECK_EQ(depth_size(MediaDepth_NONE), std::size_t(0));
}

STDROMANO_TEST_CASE(format_channel_counts)
{
    STDROMANO_CHECK_EQ(format_nchannels(MediaFormat_R), std::size_t(1));
    STDROMANO_CHECK_EQ(format_nchannels(MediaFormat_RGBA), std::size_t(4));

    MediaFormat format = MediaFormat_R;

    STDROMANO_CHECK(format_from_nchannels(3, format));
    STDROMANO_CHECK_EQ(format, MediaFormat_RGB);

    // Out of range in both directions: a 5-channel layer has no representation
    // and must be rejected rather than cast into whatever bit pattern lands.
    STDROMANO_CHECK(!format_from_nchannels(0, format));
    STDROMANO_CHECK(!format_from_nchannels(5, format));
}

STDROMANO_TEST_CASE(layer_sizes)
{
    const MediaLayer layer(1920, 1080, MediaFormat_RGBA, MediaDepth_F16);

    STDROMANO_CHECK_EQ(layer.nchannels(), std::size_t(4));
    STDROMANO_CHECK_EQ(layer.channel_size(), std::size_t(2));
    STDROMANO_CHECK_EQ(layer.pixel_size(), std::size_t(8));
    STDROMANO_CHECK_EQ(layer.x_stride(), std::size_t(8));
    STDROMANO_CHECK_EQ(layer.y_stride(), std::size_t(8 * 1920));
    STDROMANO_CHECK_EQ(layer.npixels(), std::size_t(1920) * 1080);
    STDROMANO_CHECK_EQ(layer.nbytes(), std::size_t(1920) * 1080 * 8);
    STDROMANO_CHECK(layer.is_valid());

    const MediaLayer unset(1920, 1080, MediaFormat_RGBA, MediaDepth_NONE);

    STDROMANO_CHECK(!unset.is_valid());
    STDROMANO_CHECK_EQ(unset.nbytes(), std::size_t(0));
}

// The regression this whole rework exists for: sizes come from the data window,
// never the display window. With overscan the two differ, and sizing a buffer
// from the display window means the reader writes past its end.
STDROMANO_TEST_CASE(sizes_come_from_the_data_window)
{
    const Imath::Box2i data_window(Imath::V2i(-16, -16), Imath::V2i(1935, 1095));
    const Imath::Box2i display_window(Imath::V2i(0, 0), Imath::V2i(1919, 1079));

    MediaInfo info(data_window, display_window, 1.0f);

    STDROMANO_CHECK_EQ(info.data_width(), std::uint32_t(1952));
    STDROMANO_CHECK_EQ(info.data_height(), std::uint32_t(1112));
    STDROMANO_CHECK_EQ(info.display_width(), std::uint32_t(1920));
    STDROMANO_CHECK_EQ(info.display_height(), std::uint32_t(1080));
    STDROMANO_CHECK(info.has_overscan());

    const MediaLayer& layer = info.create_layer(
        stdromano::StringD::make_ref(MediaInfo::MAIN_LAYER_NAME),
        MediaFormat_RGBA,
        MediaDepth_F16);

    STDROMANO_CHECK_EQ(layer.width(), std::uint32_t(1952));
    STDROMANO_CHECK_EQ(layer.height(), std::uint32_t(1112));
    STDROMANO_CHECK_EQ(layer.nbytes(), std::size_t(1952) * 1112 * 4 * 2);

    // And specifically: bigger than what the display window would have given.
    STDROMANO_CHECK(layer.nbytes() > std::size_t(1920) * 1080 * 4 * 2);
}

STDROMANO_TEST_CASE(layers_follow_window_changes)
{
    MediaInfo info = MediaInfo::from_size(64, 32);

    info.create_layer(stdromano::StringD::make_ref(MediaInfo::MAIN_LAYER_NAME),
                      MediaFormat_RGB,
                      MediaDepth_F32);

    STDROMANO_REQUIRE_NE(info.main(), nullptr);
    STDROMANO_CHECK_EQ(info.main()->width(), std::uint32_t(64));

    const Imath::Box2i new_window(Imath::V2i(0, 0), Imath::V2i(127, 63));
    info.set_windows(new_window, new_window);

    STDROMANO_CHECK_EQ(info.main()->width(), std::uint32_t(128));
    STDROMANO_CHECK_EQ(info.main()->height(), std::uint32_t(64));
    STDROMANO_CHECK_EQ(info.main()->nbytes(), std::size_t(128) * 64 * 3 * 4);
}

STDROMANO_TEST_CASE(layer_lookup)
{
    MediaInfo info = MediaInfo::from_size(16, 16);

    info.create_layer(stdromano::StringD::make_ref(MediaInfo::MAIN_LAYER_NAME),
                      MediaFormat_RGBA,
                      MediaDepth_F16);
    info.create_layer(stdromano::StringD::make_ref("diffuse"), MediaFormat_RGB, MediaDepth_F16);

    STDROMANO_CHECK_EQ(info.nlayers(), std::size_t(2));
    STDROMANO_CHECK(info.main() != nullptr);
    STDROMANO_CHECK(info.find_layer(stdromano::StringD::make_ref("diffuse")) != nullptr);

    // A missing layer is nullptr, not a default-constructed entry silently
    // inserted by operator[].
    STDROMANO_CHECK(info.find_layer(stdromano::StringD::make_ref("specular")) == nullptr);
    STDROMANO_CHECK_EQ(info.nlayers(), std::size_t(2));
}

STDROMANO_TEST_CASE(validity)
{
    MediaInfo empty;
    STDROMANO_CHECK(!empty.is_valid());

    MediaInfo no_main = MediaInfo::from_size(16, 16);
    no_main.create_layer(stdromano::StringD::make_ref("diffuse"), MediaFormat_RGB, MediaDepth_F16);
    STDROMANO_CHECK(!no_main.is_valid());

    MediaInfo good = MediaInfo::from_size(16, 16);
    good.create_layer(stdromano::StringD::make_ref(MediaInfo::MAIN_LAYER_NAME),
                      MediaFormat_RGB,
                      MediaDepth_F16);
    STDROMANO_CHECK(good.is_valid());
}

LOV_TEST_MAIN()
