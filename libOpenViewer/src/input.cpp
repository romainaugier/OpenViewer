// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/input.h"

#include "OpenImageIO/imageio.h"
#include "OpenEXR/ImfInputFile.h"
#include "OpenEXR/ImfFrameBuffer.h"
#include "OpenEXR/ImfHeader.h"
#include "OpenEXR/ImfChannelList.h"
#include "OpenEXR/ImfMultiPartInputFile.h"
#include "OpenEXR/ImfInputPart.h"
#include "Imath/ImathBox.h"

LOV_NAMESPACE_BEGIN

LOV_API void exr_input_func(void* __restrict buffer, 
                            const std::string& path, 
                            const InputSpecs& specs) noexcept
{
    try
    {
        // Cleanup the whole buffer from previous information
        memset(buffer, 0.0, specs.width * specs.height * specs.n_channels * specs.byte_size);

        Imf::InputFile in(path.c_str());
        Imf::FrameBuffer frame_buffer;

        const Imath::Box2i data_window = in.header().dataWindow();

        const int32_t data_width_start = data_window.min.x <= 0 ? 0 : data_window.min.x;
        const int32_t data_height_start = data_window.min.y <= 0 ? 0 : data_window.min.y;

        const int32_t data_width = data_window.max.x - data_window.min.x + 1 >= specs.width ? 
                                    specs.width : 
                                    data_window.max.x - data_window.min.x + 1;

        const int32_t data_height = data_window.max.y - data_window.min.y + 1 >= specs.height ? 
                                    specs.height : 
                                    data_window.max.y - data_window.min.y + 1;

        const Imf::PixelType pix_type = specs.byte_size == 2 ? Imf::HALF : Imf::FLOAT;

        const char* channels[] = {"R", "G", "B", "A"};

        char* char_buffer = LOVU_CAST(char*, buffer);

        uint8_t stride_offset = 0;

        for(uint8_t i = 0; i < specs.n_channels; i++)
        {   
            frame_buffer.insert(channels[i], Imf::Slice(specs.byte_size == 2 ? Imf::HALF : Imf::FLOAT,
                                                        &char_buffer[stride_offset * specs.byte_size],
                                                        specs.byte_size * specs.n_channels,
                                                        specs.byte_size * specs.width * specs.n_channels,
                                                        1, 1, 0.0f));

            stride_offset++;
        } 

        in.setFrameBuffer(frame_buffer);
        in.readPixels(data_height_start, data_height - 1);
    }
    catch(const std::exception& err)
    {
        spdlog::error("{}", err.what());
    }
}

LOV_API void png_input_func(void* __restrict buffer, 
                            const std::string& path, 
                            const InputSpecs& specs) noexcept
{
    auto in = OIIO::ImageInput::open(path);
    const OIIO::ImageSpec& spec = in->spec();
    in->read_image(spec.format, buffer);
    in->close();
}

LOV_API void jpg_input_func(void* __restrict buffer, 
                            const std::string& path, 
                            const InputSpecs& specs) noexcept
{
    auto in = OIIO::ImageInput::open(path);
    const OIIO::ImageSpec& spec = in->spec();
    in->read_image(spec.format, buffer);
    in->close();
}

LOV_API void any_input_func(void* __restrict buffer, 
                            const std::string& path, 
                            const InputSpecs& specs) noexcept
{
    auto in = OIIO::ImageInput::open(path);
    const OIIO::ImageSpec& spec = in->spec();
    in->read_image(spec.format, buffer);
    in->close();
}

LOV_NAMESPACE_END