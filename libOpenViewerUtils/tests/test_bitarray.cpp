// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewerUtils/bit_array.h"
#include "OpenViewerUtils/profiler.h"


int main(int argc, char** argv)
{
    SET_SPDLOG_FMT;
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Starting Bit Array Test");

    try
    {
        constexpr uint8_t bit_pos = 23;
        constexpr size_t arr_size = 1024;

        lovu::bit_array b(arr_size);
        spdlog::debug("Bit Array Size : {}", b.get_size());

        b.set(bit_pos);
        spdlog::debug("Set bit {} : {}", bit_pos, b.test(bit_pos));

        b.clear(bit_pos);
        spdlog::debug("Cleared bit {} : {}", bit_pos, b.test(bit_pos));

        b.resize(589);
        spdlog::debug("Resized Array : {}", b.get_size());
        
        constexpr uint16_t other_bit_pos = 467;

        b.set(other_bit_pos);
        spdlog::debug("Set bit {} : {}", other_bit_pos, b.test(other_bit_pos));

        b.clear(other_bit_pos);
        spdlog::debug("Cleared bit {} : {}", other_bit_pos, b.test(other_bit_pos));
    }
    catch(const std::exception& err)
    {
        spdlog::error("Test Failed, catched exception : \n  {}", err.what());
        
        std::exit(EXIT_FAILURE);
    }

    spdlog::info("Test passed");
    return EXIT_SUCCESS;
}