// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#include "fuzz_targets.hpp"

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv)
{
    LOV_UNUSED(argc);
    LOV_UNUSED(argv);

    lov::log::set_level(spdlog::level::off);

    return 0;
}
