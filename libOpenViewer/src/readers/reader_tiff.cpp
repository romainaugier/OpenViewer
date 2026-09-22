// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "readers/readers.hpp"

#include "OpenViewer/log.hpp"

#include "tiffio.h"

#include <cstdarg>
#include <cstdio>

LOV_NAMESPACE_BEGIN

DETAIL_NAMESPACE_BEGIN

static constexpr const char* TIFF_READER_NAME = "libtiff";
static constexpr int TIFF_HANDLER_BUF_SIZE = 1024;

static void tiff_error_handler(const char* module, const char* fmt, va_list ap)
{
    LOV_UNUSED(module);

    char buf[TIFF_HANDLER_BUF_SIZE];
    std::vsnprintf(buf, TIFF_HANDLER_BUF_SIZE, fmt, ap);

    log_error(LogCategory::ImageReader, "[{}] {}", TIFF_READER_NAME, buf);
}

static void tiff_warning_handler(const char* module, const char* fmt, va_list ap)
{
    LOV_UNUSED(module);

    char buf[TIFF_HANDLER_BUF_SIZE];
    std::vsnprintf(buf, TIFF_HANDLER_BUF_SIZE, fmt, ap);

    log_warn(LogCategory::ImageReader, "[{}] {}", TIFF_READER_NAME, buf);
}

static void install_tiff_handlers() noexcept
{
    static bool installed = []() -> bool {
        TIFFSetErrorHandler(tiff_error_handler);
        TIFFSetWarningHandler(tiff_warning_handler);
        return true;
    }();

    LOV_UNUSED(installed);
}

// What the file looks like, and whether we can read it scanline by scanline
// straight into the destination.
struct TiffDescription
{
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::uint16_t nchannels = 0;
    MediaDepth depth = MediaDepth_NONE;
    bool bottom_up = false;

    // True when the layout is not something we can copy directly: planar
    // storage, a palette, or tiles. Those go through TIFFReadRGBAImage, which
    // always produces 8-bit RGBA whatever the file held.
    bool rgba_fallback = false;
};

// Reads the description consistently for both entry points, so read_info and
// read_layer can never disagree about the format they are dealing with.
static bool describe(TIFF* tif, const stdromano::StringD& path, TiffDescription& out) noexcept
{
    std::uint32_t width = 0;
    std::uint32_t height = 0;

    if(!TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width) ||
       !TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height))
    {
        log_error(LogCategory::ImageReader,
                  "[{}] \"{}\" has no dimensions",
                  TIFF_READER_NAME,
                  path);
        return false;
    }

    std::uint16_t nchannels = 1;
    std::uint16_t bits_per_sample = 8;
    std::uint16_t sample_format = SAMPLEFORMAT_UINT;
    std::uint16_t planar_config = PLANARCONFIG_CONTIG;
    std::uint16_t photometric = PHOTOMETRIC_MINISBLACK;
    std::uint16_t orientation = ORIENTATION_TOPLEFT;

    TIFFGetFieldDefaulted(tif, TIFFTAG_SAMPLESPERPIXEL, &nchannels);
    TIFFGetFieldDefaulted(tif, TIFFTAG_BITSPERSAMPLE, &bits_per_sample);
    TIFFGetFieldDefaulted(tif, TIFFTAG_SAMPLEFORMAT, &sample_format);
    TIFFGetFieldDefaulted(tif, TIFFTAG_PLANARCONFIG, &planar_config);
    TIFFGetFieldDefaulted(tif, TIFFTAG_PHOTOMETRIC, &photometric);
    TIFFGetFieldDefaulted(tif, TIFFTAG_ORIENTATION, &orientation);

    out.width = width;
    out.height = height;
    out.bottom_up = orientation == ORIENTATION_BOTLEFT || orientation == ORIENTATION_BOTRIGHT;

    MediaDepth depth = MediaDepth_NONE;

    switch(sample_format)
    {
        case SAMPLEFORMAT_UINT:
            switch(bits_per_sample)
            {
                case 8:
                    depth = MediaDepth_U8;
                    break;
                case 16:
                    depth = MediaDepth_U16;
                    break;
                case 32:
                    depth = MediaDepth_U32;
                    break;
                default:
                    break;
            }
            break;

        case SAMPLEFORMAT_IEEEFP:
            switch(bits_per_sample)
            {
                case 16:
                    depth = MediaDepth_F16;
                    break;
                case 32:
                    depth = MediaDepth_F32;
                    break;
                default:
                    break;
            }
            break;

        default:
            break;
    }

    const bool layout_supported = planar_config == PLANARCONFIG_CONTIG &&
                                  photometric != PHOTOMETRIC_PALETTE &&
                                  !TIFFIsTiled(tif) && depth != MediaDepth_NONE &&
                                  nchannels >= 1 && nchannels <= 4;

    if(layout_supported)
    {
        out.nchannels = nchannels;
        out.depth = depth;
        out.rgba_fallback = false;
    }
    else
    {
        // TODO: tiled and planar tiffs are worth a direct path
        log_warn(LogCategory::ImageReader,
                 "[{}] \"{}\" uses a layout without a direct path "
                 "(planar={}, photometric={}, tiled={}, {} bits, {} samples); "
                 "falling back to 8-bit RGBA",
                 TIFF_READER_NAME,
                 path,
                 planar_config,
                 photometric,
                 TIFFIsTiled(tif) ? 1 : 0,
                 bits_per_sample,
                 nchannels);

        out.nchannels = 4;
        out.depth = MediaDepth_U8;
        out.rgba_fallback = true;
        out.bottom_up = false; // handled explicitly in the fallback path
    }

    return true;
}

static bool tiff_read_info(const stdromano::StringD& path, MediaInfo& info) noexcept
{
    install_tiff_handlers();

    TIFF* tif = TIFFOpen(path.c_str(), "r");

    if(tif == nullptr)
    {
        log_error(LogCategory::ImageReader, "[{}] Cannot open \"{}\"", TIFF_READER_NAME, path);
        return false;
    }

    TiffDescription desc;

    if(!describe(tif, path, desc))
    {
        TIFFClose(tif);
        return false;
    }

    TIFFClose(tif);

    MediaFormat format;

    if(!format_from_nchannels(desc.nchannels, format))
    {
        log_error(LogCategory::ImageReader,
                  "[{}] \"{}\" has {} channels, at most 4 supported",
                  TIFF_READER_NAME,
                  path,
                  desc.nchannels);
        return false;
    }

    info = MediaInfo::from_size(desc.width, desc.height);

    MediaLayer& layer = info.create_layer(stdromano::StringD::make_ref(MediaInfo::MAIN_LAYER_NAME),
                                          format,
                                          desc.depth);

    static const char* CHANNEL_NAMES[] = {"R", "G", "B", "A"};

    for(std::uint16_t i = 0; i < desc.nchannels; ++i)
        layer.add_channel(stdromano::StringD::make_from_c_str(CHANNEL_NAMES[i]));

    return true;
}

static bool tiff_read_scanlines(TIFF* tif,
                                const stdromano::StringD& path,
                                const TiffDescription& desc,
                                const MediaLayer& layer,
                                void* dst) noexcept
{
    const std::size_t y_stride = layer.y_stride();
    const tmsize_t scanline_size = TIFFScanlineSize(tif);

    if(scanline_size < 0 || static_cast<std::size_t>(scanline_size) != y_stride)
    {
        log_error(LogCategory::ImageReader,
                  "[{}] \"{}\" reports a {} byte scanline, {} expected",
                  TIFF_READER_NAME,
                  path,
                  static_cast<std::int64_t>(scanline_size),
                  y_stride);

        return false;
    }

    char* const base = static_cast<char*>(dst);

    for(std::uint32_t row = 0; row < desc.height; ++row)
    {
        const std::uint32_t dst_row = desc.bottom_up ? desc.height - 1 - row : row;

        if(TIFFReadScanline(tif, base + static_cast<std::size_t>(dst_row) * y_stride, row) < 0)
        {
            log_error(LogCategory::ImageReader,
                      "[{}] Failed to read scanline {} of \"{}\"",
                      TIFF_READER_NAME,
                      row,
                      path);
            return false;
        }
    }

    return true;
}

static bool tiff_read_rgba_fallback(TIFF* tif,
                                    const stdromano::StringD& path,
                                    const TiffDescription& desc,
                                    const MediaLayer& layer,
                                    void* dst) noexcept
{
    if(!TIFFReadRGBAImageOriented(tif,
                                  desc.width,
                                  desc.height,
                                  static_cast<std::uint32_t*>(dst),
                                  ORIENTATION_TOPLEFT,
                                  0))
    {
        log_error(LogCategory::ImageReader,
                  "[{}] Failed to read \"{}\" through the RGBA path",
                  TIFF_READER_NAME,
                  path);

        return false;
    }

    LOV_UNUSED(layer);

    return true;
}

static bool tiff_read_layer(const stdromano::StringD& path,
                            const stdromano::StringD& layer_name,
                            const MediaLayer& layer,
                            void* dst,
                            std::size_t dst_size) noexcept
{
    LOV_UNUSED(layer_name);

    if(!check_dst_size(TIFF_READER_NAME, path, layer, dst, dst_size))
        return false;

    install_tiff_handlers();

    TIFF* tif = TIFFOpen(path.c_str(), "r");

    if(tif == nullptr)
    {
        log_error(LogCategory::ImageReader, "[{}] Cannot open \"{}\"", TIFF_READER_NAME, path);
        return false;
    }

    TiffDescription desc;

    if(!describe(tif, path, desc))
    {
        TIFFClose(tif);
        return false;
    }

    if(desc.width != layer.width() || desc.height != layer.height())
    {
        log_error(LogCategory::ImageReader,
                  "[{}] \"{}\" is {}x{} but the layer expects {}x{}",
                  TIFF_READER_NAME,
                  path,
                  desc.width,
                  desc.height,
                  layer.width(),
                  layer.height());
        TIFFClose(tif);

        return false;
    }

    const bool ok = desc.rgba_fallback ?
                    tiff_read_rgba_fallback(tif, path, desc, layer, dst) :
                    tiff_read_scanlines(tif, path, desc, layer, dst);

    TIFFClose(tif);

    return ok;
}

const ImageReader g_reader_tiff = { TIFF_READER_NAME, tiff_read_info, tiff_read_layer };

DETAIL_NAMESPACE_END

LOV_NAMESPACE_END
