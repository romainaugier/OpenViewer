# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 - Present Romain Augier
# All rights reserved.

include(CheckCXXCompilerFlag)

# Architecture specific optimization flags, shared by gcc and clang. Mirrors
# stdromano's cmake/target_options.cmake: the two libraries are linked together
# and must agree on what the target machine is.
function(get_arch_compile_options out_var)
    set(arch_options)

    if(OPENVIEWER_ARCH_X86_64)
        # f16c gives hardware half <-> float conversion, which is most of the
        # cost of turning an exr into something the GPU upload path can take.
        list(APPEND arch_options -mavx2 -mfma -mf16c)
    elseif(OPENVIEWER_ARCH_AARCH64)
        if(APPLE)
            # apple-m1 is the lowest common denominator of apple silicon. It
            # already implies fp16 arithmetic (FEAT_FP16), so half conversion
            # is native on every mac we target.
            check_cxx_compiler_flag("-mcpu=apple-m1" OV_HAS_MCPU_APPLE_M1)

            if(OV_HAS_MCPU_APPLE_M1)
                list(APPEND arch_options -mcpu=apple-m1)
            endif()
        else()
            # NEON is mandatory in armv8-a. Only tune for the build machine when
            # we are not cross compiling, or the binary will not run elsewhere.
            check_cxx_compiler_flag("-mcpu=native" OV_HAS_MCPU_NATIVE)

            if(OV_HAS_MCPU_NATIVE AND NOT CMAKE_CROSSCOMPILING)
                list(APPEND arch_options -mcpu=native)
            endif()
        endif()
    endif()

    set(${out_var} ${arch_options} PARENT_SCOPE)
endfunction()

function(set_target_options target_name)
    get_arch_compile_options(ARCH_COMPILE_OPTIONS)

    # AppleClang is its own compiler id; MATCHES catches Clang and AppleClang.
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(ROMANO_CLANG 1)

        # -fsanitize=leak is not implemented on darwin; asan does leak checking
        # there on its own.
        if(APPLE)
            target_compile_options(${target_name} PRIVATE $<$<CONFIG:Debug,RelWithDebInfo>:-fsanitize=address>)
        else()
            target_compile_options(${target_name} PRIVATE $<$<CONFIG:Debug,RelWithDebInfo>:-fsanitize=leak -fsanitize=address>)
        endif()

        target_compile_options(${target_name} PRIVATE -Wall -pedantic-errors)
        target_compile_options(${target_name} PRIVATE ${ARCH_COMPILE_OPTIONS})
        target_compile_options(${target_name} PRIVATE $<$<CONFIG:Release,RelWithDebInfo>:-O3>)

        target_link_options(${target_name} PRIVATE $<$<CONFIG:Debug,RelWithDebInfo>:-fsanitize=address>)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(ROMANO_GCC 1)

        if(${ADDRSAN})
            target_compile_options(${target_name} PRIVATE $<$<CONFIG:Debug,RelWithDebInfo>:-fsanitize=address>)
            target_link_options(${target_name} PRIVATE $<$<CONFIG:Debug,RelWithDebInfo>:-fsanitize=address>)
        endif()

        if(${LEAKSAN})
            target_compile_options(${target_name} PRIVATE $<$<CONFIG:Debug,RelWithDebInfo>:-fsanitize=leak>)
            target_link_options(${target_name} PRIVATE $<$<CONFIG:Debug,RelWithDebInfo>:-fsanitize=leak>)
        endif()

        if(${UBSAN})
            target_compile_options(${target_name} PRIVATE $<$<CONFIG:Debug,RelWithDebInfo>:-fsanitize=undefined>)
            target_link_options(${target_name} PRIVATE $<$<CONFIG:Debug,RelWithDebInfo>:-fsanitize=undefined>)
        endif()

        if(${THREADSAN})
            target_compile_options(${target_name} PRIVATE $<$<CONFIG:Debug,RelWithDebInfo>:-fsanitize=thread>)
            target_link_options(${target_name} PRIVATE $<$<CONFIG:Debug,RelWithDebInfo>:-fsanitize=thread>)
        endif()

        set(COMPILE_OPTIONS -D_FORTIFY_SOURCE=2 -pipe -Wall -pedantic-errors
                            $<$<CONFIG:Release,RelWithDebInfo>:-O3>
                            ${ARCH_COMPILE_OPTIONS})

        # SVML is an x86 vector math ABI, it does not exist on arm.
        if(OPENVIEWER_ARCH_X86_64)
            list(APPEND COMPILE_OPTIONS -mveclibabi=svml)
        endif()

        target_compile_options(${target_name} PRIVATE ${COMPILE_OPTIONS})
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set(ROMANO_MSVC 1)

        if(${ADDRSAN})
            target_compile_options(${target_name} PRIVATE $<$<CONFIG:Debug,RelWithDebInfo>:/fsanitize=address>)
        endif()

        # AVX detection only makes sense on x64; MSVC on arm64 has NEON on by default.
        if(OPENVIEWER_ARCH_X86_64)
            include(find_avx)
        endif()

        # 4710 "function not inlined", 5045 "Spectre mitigation", 4324 "structure padded",
        # 4146 "unary minus on unsigned": all noise for this codebase.
        set(COMPILE_OPTIONS /W4 /wd4710 /wd5045 /wd4324 /wd4146 /utf-8 ${AVX_FLAGS} $<$<CONFIG:Release,RelWithDebInfo>:/O2 /GF /Ot /Oy /GT /GL /Oi /Zi /Gm- /Zc:inline>)

        target_compile_options(${target_name} PRIVATE ${COMPILE_OPTIONS})

        # 4300 "ignoring /INCREMENTAL because input module contains ASAN metadata"
        target_link_options(${target_name} PRIVATE /ignore:4300 /NODEFAULTLIB:library)
    endif()

    # Provides the macro definition DEBUG_BUILD
    target_compile_definitions(${target_name} PRIVATE $<$<CONFIG:Debug>:DEBUG_BUILD>)
endfunction()
