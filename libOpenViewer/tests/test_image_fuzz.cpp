// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

// Every reader fed files mutated from small valid ones. The seeds are generated
// like test_images.hpp does, for the same reasons.
//
// LOV_FUZZ_CORPUS_DIR=<dir> ./test_image_fuzz write_seed_corpus writes the seeds
// to <dir>/<extension>/ for the libFuzzer executables

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.hpp"

#include "fuzz_targets.hpp"
#include "lov_test.hpp"
#include "test_images.hpp"

#include <tiffio.h>

#include <cstdlib>
#include <filesystem>
#include <functional>

using Bytes = std::vector<std::uint8_t>;
using Corpus = std::vector<Bytes>;

static void append_to_bytes(void* context, void* data, int size)
{
    const std::uint8_t* bytes = static_cast<const std::uint8_t*>(data);
    static_cast<Bytes*>(context)->insert(static_cast<Bytes*>(context)->end(), bytes, bytes + size);
}

static Bytes gradient_u8(int width, int height, int nchannels)
{
    Bytes pixels(static_cast<std::size_t>(width * height * nchannels));

    for(std::size_t i = 0; i < pixels.size(); ++i)
        pixels[i] = static_cast<std::uint8_t>(i * 7);

    return pixels;
}

static Corpus exr_corpus()
{
    Corpus corpus;

    const auto add = [&corpus](const char* name, const std::function<void(const std::string&)>& write) {
        const std::string path = lov_test::temp_path(name);
        write(path);
        corpus.push_back(lov_test::read_file_bytes(path));
    };

    add("seed_rgba_half.exr", [](const std::string& path) {
        lov_test::write_exr(path, 8, 8, {"R", "G", "B", "A"}, Imf::HALF);
    });

    add("seed_multilayer.exr", [](const std::string& path) {
        lov_test::write_exr(path, 4, 4, {"R", "G", "B", "diffuse.R", "diffuse.G", "diffuse.B", "N.X"}, Imf::FLOAT);
    });

    add("seed_overscan.exr", [](const std::string& path) {
        const Imath::Box2i data_window(Imath::V2i(-2, -1), Imath::V2i(5, 4));
        const Imath::Box2i display_window(Imath::V2i(0, 0), Imath::V2i(3, 2));
        lov_test::write_exr(path, data_window, display_window, {"R", "G", "B"}, Imf::HALF);
    });

    return corpus;
}

static Corpus png_corpus()
{
    Corpus corpus;

    for(const int nchannels : {1, 3, 4})
    {
        Bytes png;
        const Bytes pixels = gradient_u8(8, 6, nchannels);
        stbi_write_png_to_func(append_to_bytes, &png, 8, 6, nchannels, pixels.data(), 8 * nchannels);
        corpus.push_back(std::move(png));
    }

    return corpus;
}

static Corpus jpg_corpus()
{
    Corpus corpus;

    for(const int nchannels : {1, 3})
    {
        Bytes jpg;
        const Bytes pixels = gradient_u8(16, 8, nchannels);
        stbi_write_jpg_to_func(append_to_bytes, &jpg, 16, 8, nchannels, pixels.data(), 80);
        corpus.push_back(std::move(jpg));
    }

    return corpus;
}

static Corpus hdr_corpus()
{
    std::vector<float> pixels(8 * 4 * 3);

    for(std::size_t i = 0; i < pixels.size(); ++i)
        pixels[i] = static_cast<float>(i) * 0.25f;

    Bytes hdr;
    stbi_write_hdr_to_func(append_to_bytes, &hdr, 8, 4, 3, pixels.data());

    return {hdr};
}

static Bytes write_tiff(int nchannels, int bits_per_sample, int sample_format)
{
    const std::string path = lov_test::temp_path("seed.tif");

    TIFF* tif = TIFFOpen(path.c_str(), "w");

    if(tif == nullptr)
        return {};

    constexpr std::uint32_t width = 8;
    constexpr std::uint32_t height = 4;

    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, nchannels);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bits_per_sample);
    TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, sample_format);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, nchannels >= 3 ? PHOTOMETRIC_RGB : PHOTOMETRIC_MINISBLACK);
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, height);

    if(nchannels == 4)
    {
        const std::uint16_t extra = EXTRASAMPLE_ASSOCALPHA;
        TIFFSetField(tif, TIFFTAG_EXTRASAMPLES, 1, &extra);
    }

    const std::size_t row_size = width * static_cast<std::size_t>(nchannels * bits_per_sample / 8);
    Bytes row(row_size);

    for(std::uint32_t y = 0; y < height; ++y)
    {
        for(std::size_t i = 0; i < row_size; ++i)
            row[i] = static_cast<std::uint8_t>(i * 3 + y);

        TIFFWriteScanline(tif, row.data(), y, 0);
    }

    TIFFClose(tif);

    return lov_test::read_file_bytes(path);
}

static Corpus tiff_corpus()
{
    return {write_tiff(3, 8, SAMPLEFORMAT_UINT),
            write_tiff(4, 16, SAMPLEFORMAT_UINT),
            write_tiff(1, 32, SAMPLEFORMAT_IEEEFP)};
}

struct Format
{
    const char* extension;
    Corpus (*corpus)();
    std::vector<std::string> tokens;
};

static const std::vector<Format>& formats()
{
    static const std::vector<Format> all = {
        {"exr",
         exr_corpus,
         {std::string("\x76\x2f\x31\x01", 4), "channels", "chlist", "compression", "dataWindow",
          "displayWindow", "box2i", "lineOrder", "pixelAspectRatio", "screenWindowWidth", "tiles",
          "type", "scanlineimage", "tiledimage", "deepscanline", std::string("\xff\xff\xff\x7f", 4)}},
        {"png",
         png_corpus,
         {std::string("\x89PNG\r\n\x1a\n", 8), "IHDR", "IDAT", "IEND", "PLTE", "tRNS", "gAMA",
          std::string("\x00\x00\x00\x00", 4), std::string("\xff\xff\xff\xff", 4)}},
        {"jpg",
         jpg_corpus,
         {std::string("\xff\xd8", 2), std::string("\xff\xc0", 2), std::string("\xff\xc2", 2),
          std::string("\xff\xc4", 2), std::string("\xff\xda", 2), std::string("\xff\xdb", 2),
          std::string("\xff\xd9", 2), std::string("\xff\xdd", 2)}},
        {"hdr",
         hdr_corpus,
         {"#?RADIANCE\n", "FORMAT=32-bit_rle_rgbe\n\n", "-Y ", " +X ", std::string("\x02\x02", 2), "\n"}},
        {"tif",
         tiff_corpus,
         {"II*", std::string("II*\0", 4), std::string("MM\0*", 4), std::string("\x00\x01", 2),
          std::string("\x01\x01", 2), std::string("\x02\x01", 2), std::string("\x11\x01", 2),
          std::string("\x15\x01", 2), std::string("\x53\x01", 2), std::string("\xff\xff\xff\xff", 4)}},
    };

    return all;
}

static void fuzz_format(const char* extension, std::uint64_t iterations)
{
    for(const Format& format : formats())
    {
        if(std::strcmp(format.extension, extension) != 0)
            continue;

        auto options = lov_test::fuzz_options(extension, iterations);
        options.max_input_size = 16 * 1024;
        options.corpus = format.corpus();
        options.dictionary.tokens = format.tokens;

        STDROMANO_REQUIRE_GT(options.corpus.size(), std::size_t(0));

        for(const Bytes& seed : options.corpus)
            STDROMANO_REQUIRE_GT(seed.size(), std::size_t(0));

        lov_test::QuietLogs quiet;

        const auto report = stdromano::fuzz::run_input(options, [extension](const std::uint8_t* data, std::size_t size) {
            return lov_fuzz::image_file_is_handled(extension, data, size);
        });

        LOV_REQUIRE_PROPERTY(report);

        return;
    }

    STDROMANO_FAIL("unknown format");
}

STDROMANO_TEST_CASE(seeds_are_read)
{
    for(const Format& format : formats())
    {
        for(const Bytes& seed : format.corpus())
        {
            const std::string path = lov_test::temp_path("seed_check.") + format.extension;

            STDROMANO_REQUIRE(lov_test::write_file_bytes(path, seed.data(), seed.size()));

            lov::MediaInfo info;
            STDROMANO_CHECK_MSG(lov::image_read_info(stdromano::StringD::make_from_c_str(path.c_str()), info),
                                format.extension);
            STDROMANO_CHECK_MSG(info.is_valid(), format.extension);
        }
    }
}

STDROMANO_TEST_CASE(fuzz_exr)
{
    fuzz_format("exr", 1000);
}

STDROMANO_TEST_CASE(fuzz_png)
{
    fuzz_format("png", 1000);
}

STDROMANO_TEST_CASE(fuzz_jpg)
{
    fuzz_format("jpg", 1000);
}

STDROMANO_TEST_CASE(fuzz_hdr)
{
    fuzz_format("hdr", 1000);
}

STDROMANO_TEST_CASE(fuzz_tif)
{
    fuzz_format("tif", 1000);
}

STDROMANO_TEST_CASE(write_seed_corpus)
{
    const char* root = std::getenv("LOV_FUZZ_CORPUS_DIR");

    if(root == nullptr || root[0] == '\0')
        return;

    for(const Format& format : formats())
    {
        const std::filesystem::path dir = std::filesystem::path(root) / format.extension;
        std::filesystem::create_directories(dir);

        std::size_t index = 0;

        for(const Bytes& seed : format.corpus())
        {
            const std::string path = (dir / ("seed_" + std::to_string(index++))).string();
            STDROMANO_CHECK(lov_test::write_file_bytes(path, seed.data(), seed.size()));
        }
    }
}

LOV_TEST_MAIN()
