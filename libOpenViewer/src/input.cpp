// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/input.h"

#include "OpenImageIO/imageio.h"

LOV_NAMESPACE_BEGIN

LOV_DLL void exr_input_func(void* __restrict buffer, const std::string& path) noexcept
{
    auto in = OIIO::ImageInput::open(path);
    const OIIO::ImageSpec& spec = in->spec();
    in->read_image(0, -1, spec.format, buffer);
    in->close();

}

LOV_DLL void png_input_func(void* __restrict buffer, const std::string& path) noexcept
{
    auto in = OIIO::ImageInput::open(path);
    const OIIO::ImageSpec& spec = in->spec();
    in->read_image(0, -1, spec.format, buffer);
    in->close();
}

LOV_DLL void jpg_input_func(void* __restrict buffer, const std::string& path) noexcept
{
    auto in = OIIO::ImageInput::open(path);
    const OIIO::ImageSpec& spec = in->spec();
    in->read_image(0, -1, spec.format, buffer);
    in->close();
}

LOV_DLL void any_input_func(void* __restrict buffer, const std::string& path) noexcept
{
    auto in = OIIO::ImageInput::open(path);
    const OIIO::ImageSpec& spec = in->spec();
    in->read_image(0, -1, spec.format, buffer);
    in->close();
}

LOV_NAMESPACE_END