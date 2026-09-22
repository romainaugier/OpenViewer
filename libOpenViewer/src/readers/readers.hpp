// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__LOV_READERS)
#define __LOV_READERS

#include "OpenViewer/image_reader.hpp"

LOV_NAMESPACE_BEGIN

DETAIL_NAMESPACE_BEGIN

extern const ImageReader g_reader_exr;
extern const ImageReader g_reader_tiff;
extern const ImageReader g_reader_stb_ldr; // jpeg, png, bmp, tga
extern const ImageReader g_reader_stb_hdr; // radiance .hdr

bool check_dst_size(const char* reader_name,
                    const stdromano::StringD& path,
                    const MediaLayer& layer,
                    const void* dst,
                    std::size_t dst_size) noexcept;

DETAIL_NAMESPACE_END

LOV_NAMESPACE_END

#endif // !defined(__LOV_READERS)
