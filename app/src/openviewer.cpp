// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/image_reader.hpp"
#include "OpenViewer/log.hpp"
#include "OpenViewer/media.hpp"

// Placeholder front-end until the display layer lands (W6). It exists so that
// the library is exercised by something other than the tests, and so that the
// reader path can be checked against a real plate from the command line.
int main(int argc, char** argv)
{
    lov::log::initialize(spdlog::level::debug);

    if(argc < 2)
    {
        lov::log_info("usage: {} <image>", argv[0]);
        lov::log_info("supported extensions:");

        for(const auto& ext : lov::ImageReaderRegistry::get_instance().supported_extensions())
        {
            lov::log_info("  .{}", ext);
        }

        return 0;
    }

    lov::ImageMedia media(stdromano::StringD::make_from_c_str(argv[1]));

    if(!media.open())
    {
        return 1;
    }

    media.debug();

    return 0;
}
