// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.
//
// Synthetic images for the reader tests.
//
// Generated at run time rather than committed: binary fixtures in git go stale,
// nobody can tell what is inside them, and a repository that ships test plates
// stops being cloneable over a hotel connection. The generator here is also the
// specification of what the reader is expected to produce.

#pragma once

#if !defined(__LOV_TEST_IMAGES)
#define __LOV_TEST_IMAGES

#include "OpenViewer/media_info.hpp"

#include "OpenEXR/ImfChannelList.h"
#include "OpenEXR/ImfFrameBuffer.h"
#include "OpenEXR/ImfHeader.h"
#include "OpenEXR/ImfOutputFile.h"

#include <cstring>
#include <string>
#include <vector>

namespace lov_test
{

// Deterministic pixel value, in absolute (data-window) coordinates.
//
// Every value is an integer below 1024. half represents integers exactly only
// up to 2048 (past that its spacing is 2), so staying under 1024 keeps every
// value exact in half and float and lets a round-trip be compared with ==.
// Depending on an epsilon here would hide exactly the kind of off-by-one
// addressing bug these tests exist to catch.
//
// 3 bits of x, 3 bits of y, 3 bits of channel (max 511): neighbouring pixels,
// rows and up to 8 file channels all differ, so a stride mistake, a channel
// order mistake, or reading the main layer instead of an AOV cannot produce the
// right value by accident.
inline float expected_pixel(int x, int y, int channel) noexcept
{
    return static_cast<float>((x & 7) + ((y & 7) << 3) + ((channel & 7) << 6));
}

// Writes a scanline exr whose data window may differ from its display window.
//
// `channel_names` are written in the order given, which lets a test write them
// out of canonical order and check that the reader puts them back.
inline void write_exr(const std::string& path,
                      const Imath::Box2i& data_window,
                      const Imath::Box2i& display_window,
                      const std::vector<std::string>& channel_names,
                      Imf::PixelType pixel_type)
{
    const int width = data_window.max.x - data_window.min.x + 1;
    const int height = data_window.max.y - data_window.min.y + 1;
    const std::size_t nchannels = channel_names.size();

    Imf::Header header(display_window, data_window, 1.0f);

    for(const auto& name : channel_names)
    {
        header.channels().insert(name, Imf::Channel(pixel_type));
    }

    // Staged in the channel's own type: Imf converts between pixel types when
    // reading, but not when writing, and a float slice feeding a half channel
    // throws.
    const std::size_t sample_size = pixel_type == Imf::HALF ? sizeof(half) : sizeof(float);
    const std::size_t npixels = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);

    std::vector<char> pixels(npixels * nchannels * sample_size);

    for(int y = 0; y < height; ++y)
    {
        for(int x = 0; x < width; ++x)
        {
            for(std::size_t c = 0; c < nchannels; ++c)
            {
                const std::size_t index =
                    (static_cast<std::size_t>(y) * static_cast<std::size_t>(width) +
                     static_cast<std::size_t>(x)) *
                        nchannels +
                    c;

                // Absolute coordinates: the value of a pixel does not depend on
                // where the data window happens to start.
                const float value = expected_pixel(x + data_window.min.x,
                                                   y + data_window.min.y,
                                                   static_cast<int>(c));

                if(pixel_type == Imf::HALF)
                {
                    const half h(value);
                    std::memcpy(pixels.data() + index * sample_size, &h, sizeof(half));
                }
                else
                {
                    std::memcpy(pixels.data() + index * sample_size, &value, sizeof(float));
                }
            }
        }
    }

    const std::size_t x_stride = sample_size * nchannels;
    const std::size_t y_stride = x_stride * static_cast<std::size_t>(width);

    char* const origin = pixels.data() -
                         static_cast<std::ptrdiff_t>(data_window.min.x) *
                             static_cast<std::ptrdiff_t>(x_stride) -
                         static_cast<std::ptrdiff_t>(data_window.min.y) *
                             static_cast<std::ptrdiff_t>(y_stride);

    Imf::FrameBuffer frame_buffer;

    for(std::size_t c = 0; c < nchannels; ++c)
    {
        frame_buffer.insert(channel_names[c],
                            Imf::Slice(pixel_type,
                                       origin + c * sample_size,
                                       x_stride,
                                       y_stride));
    }

    Imf::OutputFile file(path.c_str(), header);
    file.setFrameBuffer(frame_buffer);
    file.writePixels(height);
}

// The common case: data window == display window, origin at (0, 0).
inline void write_exr(const std::string& path,
                      int width,
                      int height,
                      const std::vector<std::string>& channel_names,
                      Imf::PixelType pixel_type)
{
    const Imath::Box2i window(Imath::V2i(0, 0), Imath::V2i(width - 1, height - 1));

    write_exr(path, window, window, channel_names, pixel_type);
}

} // namespace lov_test

#endif // !defined(__LOV_TEST_IMAGES)
