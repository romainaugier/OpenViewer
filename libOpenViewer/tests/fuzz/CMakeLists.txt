# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 - Present Romain Augier
# All rights reserved.

# libFuzzer executables (OPENVIEWER_BUILD_FUZZERS=ON, Clang only), running the
# targets of ../fuzz_targets.hpp without an iteration limit:
#
#     LOV_FUZZ_CORPUS_DIR=corpus ./test_image_fuzz write_seed_corpus
#     ./fuzz_image_exr corpus/exr -max_len=16384

include(target_options)

function(add_openviewer_fuzzer name source)
    add_executable(${name} ${source})

    set_target_properties(${name} PROPERTIES CXX_STANDARD 17
                                             CXX_STANDARD_REQUIRED ON)

    target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                                               ${CMAKE_CURRENT_SOURCE_DIR}/..)

    target_compile_options(${name} PRIVATE -fsanitize=fuzzer,address,undefined -g)
    target_link_options(${name} PRIVATE -fsanitize=fuzzer,address,undefined)

    target_link_libraries(${name} PRIVATE ${OPENVIEWER_LIBS})
endfunction()

add_openviewer_fuzzer(fuzz_sequence_pattern fuzz_sequence_pattern.cpp)
add_openviewer_fuzzer(fuzz_log_levels fuzz_log_levels.cpp)
add_openviewer_fuzzer(fuzz_media_cache fuzz_media_cache.cpp)

foreach(extension exr png jpg hdr tif)
    add_openviewer_fuzzer(fuzz_image_${extension} fuzz_image.cpp)
    target_compile_definitions(fuzz_image_${extension} PRIVATE LOV_FUZZ_EXTENSION="${extension}")
endforeach()
