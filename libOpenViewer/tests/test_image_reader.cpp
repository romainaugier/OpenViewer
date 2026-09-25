// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "lov_test.hpp"

#include "OpenViewer/image_reader.hpp"

using namespace lov;

STDROMANO_TEST_CASE(builtin_readers_are_registered)
{
    const ImageReaderRegistry& registry = ImageReaderRegistry::get_instance();

    STDROMANO_CHECK(registry.find(stdromano::StringD::make_ref("exr")) != nullptr);
    STDROMANO_CHECK(registry.find(stdromano::StringD::make_ref("tif")) != nullptr);
    STDROMANO_CHECK(registry.find(stdromano::StringD::make_ref("tiff")) != nullptr);
    STDROMANO_CHECK(registry.find(stdromano::StringD::make_ref("jpg")) != nullptr);
    STDROMANO_CHECK(registry.find(stdromano::StringD::make_ref("png")) != nullptr);
    STDROMANO_CHECK(registry.find(stdromano::StringD::make_ref("hdr")) != nullptr);

    STDROMANO_CHECK(registry.find(stdromano::StringD::make_ref("psd")) == nullptr);
}

STDROMANO_TEST_CASE(extension_matching_is_forgiving)
{
    const ImageReaderRegistry& registry = ImageReaderRegistry::get_instance();

    const ImageReader* reader = registry.find(stdromano::StringD::make_ref("exr"));

    STDROMANO_REQUIRE_NE(reader, nullptr);

    STDROMANO_CHECK_EQ(registry.find(stdromano::StringD::make_ref("EXR")), reader);
    STDROMANO_CHECK_EQ(registry.find(stdromano::StringD::make_ref(".exr")), reader);
    STDROMANO_CHECK_EQ(registry.find(stdromano::StringD::make_ref(".EXR")), reader);
}

STDROMANO_TEST_CASE(lookup_by_path)
{
    const ImageReaderRegistry& registry = ImageReaderRegistry::get_instance();

    STDROMANO_CHECK(registry.find_for_path(stdromano::StringD::make_ref("/a/b/render.1001.exr")) !=
              nullptr);
    STDROMANO_CHECK(registry.find_for_path(
                  stdromano::StringD::make_ref("C:/shots/sh010/plate.0001.EXR")) != nullptr);

    STDROMANO_CHECK(registry.find_for_path(stdromano::StringD::make_ref("/a/b/notes.txt")) == nullptr);

    STDROMANO_CHECK(image_is_supported(stdromano::StringD::make_ref("plate.1001.exr")));
    STDROMANO_CHECK(!image_is_supported(stdromano::StringD::make_ref("plate.1001.mov")));
}

STDROMANO_TEST_CASE(unknown_extension_fails_cleanly)
{
    MediaInfo info;

    // No crash, no exception, just false. Callers walking a directory rely on
    // this rather than on the file dialog having filtered correctly.
    STDROMANO_CHECK(!image_read_info(stdromano::StringD::make_ref("/nonexistent/file.psd"), info));
    STDROMANO_CHECK(!image_read_info(stdromano::StringD::make_ref("/nonexistent/file.exr"), info));
    STDROMANO_CHECK(!image_read_info(stdromano::StringD::make_ref("no_extension_at_all"), info));
}

static bool fake_read_info(const stdromano::StringD& path, MediaInfo& info) noexcept
{
    LOV_UNUSED(path);

    info = MediaInfo::from_size(4, 4);
    info.create_layer(stdromano::StringD::make_ref(MediaInfo::MAIN_LAYER_NAME),
                      MediaFormat_RGBA,
                      MediaDepth_U8);

    return true;
}

static bool fake_read_layer(const stdromano::StringD& path,
                            const stdromano::StringD& layer_name,
                            const MediaLayer& layer,
                            void* dst,
                            std::size_t dst_size) noexcept
{
    LOV_UNUSED(path);
    LOV_UNUSED(layer_name);

    if(dst_size < layer.nbytes())
    {
        return false;
    }

    std::memset(dst, 0x7f, layer.nbytes());

    return true;
}

static const ImageReader g_fake_reader = {"fake", fake_read_info, fake_read_layer};

STDROMANO_TEST_CASE(custom_readers_can_be_registered)
{
    ImageReaderRegistry& registry = ImageReaderRegistry::get_instance();

    registry.register_reader(stdromano::StringD::make_ref("fake"), &g_fake_reader);

    MediaInfo info;
    STDROMANO_REQUIRE(image_read_info(stdromano::StringD::make_ref("whatever.fake"), info));
    STDROMANO_REQUIRE_NE(info.main(), nullptr);
    STDROMANO_CHECK_EQ(info.main()->nbytes(), std::size_t(4 * 4 * 4));

    std::vector<char> buffer(info.main()->nbytes());

    STDROMANO_CHECK(image_read_layer(stdromano::StringD::make_ref("whatever.fake"),
                               stdromano::StringD::make_ref(MediaInfo::MAIN_LAYER_NAME),
                               *info.main(),
                               buffer.data(),
                               buffer.size()));

    STDROMANO_CHECK_EQ(static_cast<unsigned char>(buffer[0]), 0x7f);
}

STDROMANO_TEST_CASE(incomplete_readers_are_refused)
{
    ImageReaderRegistry& registry = ImageReaderRegistry::get_instance();

    static const ImageReader broken = {"broken", nullptr, nullptr};

    registry.register_reader(stdromano::StringD::make_ref("broken"), &broken);

    STDROMANO_CHECK(registry.find(stdromano::StringD::make_ref("broken")) == nullptr);
}

LOV_TEST_MAIN()
