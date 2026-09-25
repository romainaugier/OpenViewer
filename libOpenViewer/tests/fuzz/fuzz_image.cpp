// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

// Built once per format, LOV_FUZZ_EXTENSION picks the reader

#include "fuzz_main.hpp"

static bool fuzz_image(const std::uint8_t* data, std::size_t size)
{
    return lov_fuzz::image_file_is_handled(LOV_FUZZ_EXTENSION, data, size);
}

STDROMANO_FUZZ_LIBFUZZER_INPUT(fuzz_image)
