// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "lov_test.hpp"

#include "OpenViewer/image_reader.hpp"

using namespace lov;

LOV_TEST(builtin_readers_are_registered)
{
    const ImageReaderRegistry& registry = ImageReaderRegistry::get_instance();

    LOV_CHECK(registry.find(stdromano::StringD::make_ref("exr")) != nullptr);
    LOV_CHECK(registry.find(stdromano::StringD::make_ref("tif")) != nullptr);
    LOV_CHECK(registry.find(stdromano::StringD::make_ref("tiff")) != nullptr);
    LOV_CHECK(registry.find(stdromano::StringD::make_ref("jpg")) != nullptr);
    LOV_CHECK(registry.find(stdromano::StringD::make_ref("png")) != nullptr);
    LOV_CHECK(registry.find(stdromano::StringD::make_ref("hdr")) != nullptr);

    LOV_CHECK(registry.find(stdromano::StringD::make_ref("psd")) == nullptr);
}

LOV_TEST(extension_matching_is_forgiving)
{
    const ImageReaderRegistry& registry = ImageReaderRegistry::get_instance();

    const ImageReader* reader = registry.find(stdromano::StringD::make_ref("exr"));

    LOV_REQUIRE(reader != nullptr);

    LOV_CHECK_EQ(registry.find(stdromano::StringD::make_ref("EXR")), reader);
    LOV_CHECK_EQ(registry.find(stdromano::StringD::make_ref(".exr")), reader);
    LOV_CHECK_EQ(registry.find(stdromano::StringD::make_ref(".EXR")), reader);
}

LOV_TEST(lookup_by_path)
{
    const ImageReaderRegistry& registry = ImageReaderRegistry::get_instance();

    LOV_CHECK(registry.find_for_path(stdromano::StringD::make_ref("/a/b/render.1001.exr")) !=
              nullptr);
    LOV_CHECK(registry.find_for_path(
                  stdromano::StringD::make_ref("C:/shots/sh010/plate.0001.EXR")) != nullptr);

    LOV_CHECK(registry.find_for_path(stdromano::StringD::make_ref("/a/b/notes.txt")) == nullptr);

    LOV_CHECK(image_is_supported(stdromano::StringD::make_ref("plate.1001.exr")));
    LOV_CHECK(!image_is_supported(stdromano::StringD::make_ref("plate.1001.mov")));
}

LOV_TEST(unknown_extension_fails_cleanly)
{
    MediaInfo info;

    // No crash, no exception, just false. Callers walking a directory rely on
    // this rather than on the file dialog having filtered correctly.
    LOV_CHECK(!image_read_info(stdromano::StringD::make_ref("/nonexistent/file.psd"), info));
    LOV_CHECK(!image_read_info(stdromano::StringD::make_ref("/nonexistent/file.exr"), info));
    LOV_CHECK(!image_read_info(stdromano::StringD::make_ref("no_extension_at_all"), info));
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

LOV_TEST(custom_readers_can_be_registered)
{
    ImageReaderRegistry& registry = ImageReaderRegistry::get_instance();

    registry.register_reader(stdromano::StringD::make_ref("fake"), &g_fake_reader);

    MediaInfo info;
    LOV_REQUIRE(image_read_info(stdromano::StringD::make_ref("whatever.fake"), info));
    LOV_REQUIRE(info.main() != nullptr);
    LOV_CHECK_EQ(info.main()->nbytes(), std::size_t(4 * 4 * 4));

    std::vector<char> buffer(info.main()->nbytes());

    LOV_CHECK(image_read_layer(stdromano::StringD::make_ref("whatever.fake"),
                               stdromano::StringD::make_ref(MediaInfo::MAIN_LAYER_NAME),
                               *info.main(),
                               buffer.data(),
                               buffer.size()));

    LOV_CHECK_EQ(static_cast<int>(static_cast<unsigned char>(buffer[0])), 0x7f);
}

LOV_TEST(incomplete_readers_are_refused)
{
    ImageReaderRegistry& registry = ImageReaderRegistry::get_instance();

    static const ImageReader broken = {"broken", nullptr, nullptr};

    registry.register_reader(stdromano::StringD::make_ref("broken"), &broken);

    LOV_CHECK(registry.find(stdromano::StringD::make_ref("broken")) == nullptr);
}

LOV_TEST_MAIN()
