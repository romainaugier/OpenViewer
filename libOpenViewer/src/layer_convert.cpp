// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.
//
#include "OpenViewer/image.hpp"

#include "stdromano/simd.hpp"

#include <intrin.h>

#include <type_traits>

LOV_NAMESPACE_BEGIN

/******************************************/
/* Utilities */
/******************************************/

template<typename From, typename To>
struct is_float_to_integral : std::conjunction<std::is_same<From, float>,
                                               std::is_integral<To>> {};

template<typename From, typename To>
struct is_float_to_half : std::conjunction<std::is_same<From, float>,
                                           std::is_same<To, half>> {};

template<typename From, typename To>
struct is_half_to_integral : std::conjunction<std::is_same<From, half>,
                                              std::is_integral<To>> {};

template<typename From, typename To>
struct is_half_to_float : std::conjunction<std::is_same<From, half>,
                                           std::is_same<To, float>> {};

template<typename From, typename To>
struct is_integral_to_integral : std::conjunction<std::is_integral<From>,
                                                  std::is_integral<To>> {};

template<typename From, typename To>
struct is_integral_to_float : std::conjunction<std::is_integral<From>,
                                               std::is_same<To, float>> {};

template<typename From, typename To>
struct is_integral_to_half : std::conjunction<std::is_integral<From>,
                                              std::is_same<To, half>> {};

template<typename From, typename To>
inline constexpr bool is_float_to_integral_v = is_float_to_integral<From, To>::value;

template<typename From, typename To>
inline constexpr bool is_float_to_half_v = is_float_to_half<From, To>::value;

template<typename From, typename To>
inline constexpr bool is_half_to_integral_v = is_half_to_integral<From, To>::value;

template<typename From, typename To>
inline constexpr bool is_half_to_float_v = is_half_to_float<From, To>::value;

template<typename From, typename To>
inline constexpr bool is_integral_to_integral_v = is_integral_to_integral<From, To>::value;

template<typename From, typename To>
inline constexpr bool is_integral_to_float_v = is_integral_to_float<From, To>::value;

template<typename From, typename To>
inline constexpr bool is_integral_to_half_v = is_integral_to_half<From, To>::value;

/******************************************/
/* Scalar */
/******************************************/

template<typename From, typename To>
std::enable_if_t<std::is_same_v<From, To>>
layer_convert_scalar_kernel(const From* __restrict from,
                            To* __restrict to,
                            const std::size_t size) noexcept
{
    LOV_UNUSED(from);
    LOV_UNUSED(to);
    LOV_UNUSED(size);
}

template<typename From, typename To>
std::enable_if_t<is_float_to_integral_v<From, To>>
layer_convert_scalar_kernel(const From* __restrict from,
                            To* __restrict to,
                            const std::size_t size) noexcept
{
    constexpr float m = static_cast<float>(std::numeric_limits<To>::max());

    for(std::size_t i = 0; i < size; ++i)
    {
        to[i] = static_cast<To>(std::clamp(from[i], 0.0f, 1.0f) * m);
    }
}

template<typename From, typename To>
std::enable_if_t<is_float_to_half_v<From, To>>
layer_convert_scalar_kernel(const From* __restrict from,
                            To* __restrict to,
                            const std::size_t size) noexcept
{
    for(std::size_t i = 0; i < size; ++i)
    {
        to[i] = static_cast<half>(from[i]);
    }
}

template<typename From, typename To>
std::enable_if_t<is_half_to_integral_v<From, To>>
layer_convert_scalar_kernel(const From* __restrict from,
                            To* __restrict to,
                            const std::size_t size) noexcept
{
    const float m = static_cast<float>(std::numeric_limits<To>::max());

    for(std::size_t i = 0; i < size; ++i)
    {
        to[i] = static_cast<To>(std::clamp(static_cast<float>(from[i]), 0.0f, 1.0f) * m);
    }
}

template<typename From, typename To>
std::enable_if_t<is_half_to_float_v<From, To>>
layer_convert_scalar_kernel(const From* __restrict from,
                            To* __restrict to,
                            const std::size_t size) noexcept
{
    for(std::size_t i = 0; i < size; ++i)
    {
        to[i] = static_cast<float>(from[i]);
    }
}

template<typename From, typename To>
std::enable_if_t<is_integral_to_integral_v<From, To>>
layer_convert_scalar_kernel(const From* __restrict from,
                            To* __restrict to,
                            const std::size_t size) noexcept
{
    constexpr float inv_m_f = 1.0f / static_cast<float>(std::numeric_limits<From>::max());
    constexpr float m_t = static_cast<float>(std::numeric_limits<To>::max());

    for(std::size_t i = 0; i < size; ++i)
    {
        to[i] = static_cast<To>(static_cast<float>(from[i]) * inv_m_f * m_t);
    }
}

template<typename From, typename To>
std::enable_if_t<is_integral_to_float_v<From, To>>
layer_convert_scalar_kernel(const From* __restrict from,
                            To* __restrict to,
                            const std::size_t size) noexcept
{
    constexpr float inv_m = 1.0f / static_cast<float>(std::numeric_limits<From>::max());

    for(std::size_t i = 0; i < size; ++i)
    {
        to[i] = static_cast<float>(from[i]) * inv_m;
    }
}

template<typename From, typename To>
std::enable_if_t<is_integral_to_half_v<From, To>>
layer_convert_scalar_kernel(const From* __restrict from,
                            To* __restrict to,
                            const std::size_t size) noexcept
{
    const float inv_m = 1.0f / static_cast<float>(std::numeric_limits<From>::max());

    for(std::size_t i = 0; i < size; ++i)
    {
        to[i] = static_cast<half>(static_cast<float>(from[i]) * inv_m);
    }
}

template<std::uint8_t from_depth>
struct ScalarDispatcher
{
    template<std::uint8_t to_depth>
    static void dispatch_to(const void* __restrict from,
                            void* __restrict to, const std::size_t size) noexcept
    {
        using FromType = depth_to_type_t<from_depth>;
        using ToType = depth_to_type_t<to_depth>;

        if constexpr (std::is_same_v<FromType, ToType>)
        {
            return;
        }
        else
        {
            layer_convert_scalar_kernel(static_cast<const FromType*>(from),
                                    static_cast<ToType*>(to),
                                    size);
        }
    }

    static void dispatch(const void* __restrict from,
                         void* __restrict to,
                         std::uint8_t to_depth,
                         std::size_t size) noexcept
    {
        switch(to_depth)
        {
            case LayerDepth_U8:
                ScalarDispatcher::dispatch_to<LayerDepth_U8>(from, to, size);
                break;
            case LayerDepth_U16:
                ScalarDispatcher::dispatch_to<LayerDepth_U16>(from, to, size);
                break;
            case LayerDepth_U32:
                ScalarDispatcher::dispatch_to<LayerDepth_U32>(from, to, size);
                break;
            case LayerDepth_F16:
                ScalarDispatcher::dispatch_to<LayerDepth_F16>(from, to, size);
                break;
            case LayerDepth_F32:
                ScalarDispatcher::dispatch_to<LayerDepth_F32>(from, to, size);
                break;
        }
    }
};

void layer_convert_scalar(const void* __restrict from,
                          void* __restrict to,
                          std::uint8_t from_depth,
                          std::uint8_t to_depth,
                          std::size_t size) noexcept
{
    switch(from_depth)
    {
        case LayerDepth_U8:
            ScalarDispatcher<LayerDepth_U8>::dispatch(from, to, to_depth, size);
            break;
        case LayerDepth_U16:
            ScalarDispatcher<LayerDepth_U16>::dispatch(from, to, to_depth, size);
            break;
        case LayerDepth_U32:
            ScalarDispatcher<LayerDepth_U32>::dispatch(from, to, to_depth, size);
            break;
        case LayerDepth_F16:
            ScalarDispatcher<LayerDepth_F16>::dispatch(from, to, to_depth, size);
            break;
        case LayerDepth_F32:
            ScalarDispatcher<LayerDepth_F32>::dispatch(from, to, to_depth, size);
            break;
    }
}

/******************************************/
/* SSE */
/******************************************/

template<typename From, typename To>
std::enable_if_t<std::is_same_v<From, To>>
layer_convert_sse_kernel(const From* __restrict from,
                         To* __restrict to,
                         const std::size_t size) noexcept
{
    LOV_UNUSED(from);
    LOV_UNUSED(to);
    LOV_UNUSED(size);
}

template<typename From, typename To>
std::enable_if_t<is_float_to_integral_v<From, To>>
layer_convert_sse_kernel(const From* __restrict from,
                         To* __restrict to,
                         const std::size_t size) noexcept
{
    constexpr std::size_t width = 4;

    constexpr float m = static_cast<float>(std::numeric_limits<To>::max());

    const __m128 m_128 = _mm_set1_ps(m);
    const __m128 zeros = _mm_set1_ps(0.0f);
    const __m128 ones = _mm_set1_ps(1.0f);

    std::size_t i = 0;

    for(; i < size; i += width)
    {
        __m128 f = _mm_load_ps(std::addressof(from[i]));

        f = _mm_max_ps(f, zeros);
        f = _mm_min_ps(f, ones);
        f = _mm_mul_ps(f, m_128);

        if constexpr (std::is_same_v<To, std::uint8_t>)
        {
            const __m128i i_values = _mm_cvtps_epi32(f);
            const __m128i s_values = _mm_packs_epi32(i_values, i_values);
            const __m128i res = _mm_packus_epi16(s_values, s_values);

            _mm_storeu_si32(std::addressof(to[i]), res);
        }
        else if constexpr (std::is_same_v<To, std::uint16_t>)
        {
            const __m128i i_values = _mm_cvtps_epi32(f);
            const __m128i s_values = _mm_packus_epi32(i_values, i_values);

            _mm_storeu_si64(std::addressof(to[i]), s_values);
        }
        else if constexpr (std::is_same_v<To, std::uint32_t>)
        {
            for(; i < size; ++i)
            {
                to[i] = static_cast<To>(std::clamp(from[i], 0.0f, 1.0f) * m);
            }
        }
        else
        {
            static_assert(0, "To integral type not supported");
        }
    }

    for(; i < size; ++i)
    {
        to[i] = static_cast<To>(std::clamp(from[i], 0.0f, 1.0f) * m);
    }
}

template<typename From, typename To>
std::enable_if_t<is_float_to_half_v<From, To>>
layer_convert_sse_kernel(const From* __restrict from,
                         To* __restrict to,
                         const std::size_t size) noexcept
{
    if(stdromano::simd_has_f16c())
    {
        constexpr std::size_t width = 4;

        std::size_t i = 0;

        for(; i < size; i += width)
        {
            const __m128 f = _mm_load_ps(std::addressof(from[i]));
            const __m128i h = _mm_cvtps_ph(f, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);

            _mm_storeu_si64(std::addressof(to[i]), h);
        }

        for(; i < size; ++i)
        {
            to[i] = static_cast<half>(from[i]);
        }
    }
    else
    {
        for(std::size_t i = 0; i < size; ++i)
        {
            to[i] = static_cast<half>(from[i]);
        }
    }
}

template<typename From, typename To>
std::enable_if_t<is_half_to_integral_v<From, To>>
layer_convert_sse_kernel(const From* __restrict from,
                         To* __restrict to,
                         const std::size_t size) noexcept
{
    constexpr float m = static_cast<float>(std::numeric_limits<To>::max());

    if(stdromano::simd_has_f16c())
    {
        constexpr std::size_t width = 4;

        const __m128 m_128 = _mm_set1_ps(m);
        const __m128 zeros = _mm_set1_ps(0.0f);
        const __m128 ones = _mm_set1_ps(1.0f);

        std::size_t i = 0;

        for(; i < size; i += width)
        {
            const __m128i h = _mm_loadu_si64(std::addressof(from[i]));
            __m128 f = _mm_cvtph_ps(h);

            f = _mm_max_ps(f, zeros);
            f = _mm_min_ps(f, ones);
            f = _mm_mul_ps(f, m_128);

            if constexpr (std::is_same_v<To, std::uint8_t>)
            {
                const __m128i i_values = _mm_cvtps_epi32(f);
                const __m128i s_values = _mm_packs_epi32(i_values, i_values);
                const __m128i res = _mm_packus_epi16(s_values, s_values);

                _mm_storeu_si32(std::addressof(to[i]), res);
            }
            else if constexpr (std::is_same_v<To, std::uint16_t>)
            {
                const __m128i i_values = _mm_cvtps_epi32(f);
                const __m128i s_values = _mm_packus_epi32(i_values, i_values);

                _mm_storeu_si64(std::addressof(to[i]), s_values);
            }
            else if constexpr (std::is_same_v<To, std::uint32_t>)
            {
                for(; i < size; ++i)
                {
                    to[i] = static_cast<To>(std::clamp(static_cast<float>(from[i]), 0.0f, 1.0f) * m);
                }
            }
            else
            {
                static_assert(0, "To integral type not supported");
            }
        }

        for(; i < size; ++i)
        {
            to[i] = static_cast<To>(std::clamp(static_cast<float>(from[i]), 0.0f, 1.0f) * m);
        }
    }
    else
    {
        for(std::size_t i = 0; i < size; ++i)
        {
            to[i] = static_cast<To>(std::clamp(static_cast<float>(from[i]), 0.0f, 1.0f) * m);
        }
    }
}

template<typename From, typename To>
std::enable_if_t<is_half_to_float_v<From, To>>
layer_convert_sse_kernel(const From* __restrict from,
                         To* __restrict to,
                         const std::size_t size) noexcept
{
    if(stdromano::simd_has_f16c())
    {
        constexpr std::size_t width = 4;

        std::size_t i = 0;

        for(; i < size; i += width)
        {
            const __m128i h = _mm_loadu_si64(std::addressof(from[i]));
            const __m128 f = _mm_cvtph_ps(h);

            _mm_store_ps(std::addressof(to[i]), f);
        }

        for(; i < size; ++i)
        {
            to[i] = static_cast<float>(from[i]);
        }
    }
    else
    {
        for(std::size_t i = 0; i < size; ++i)
        {
            to[i] = static_cast<float>(from[i]);
        }
    }
}

template<typename From, typename To>
std::enable_if_t<is_integral_to_integral_v<From, To>>
layer_convert_sse_kernel(const From* __restrict from,
                         To* __restrict to,
                         const std::size_t size) noexcept
{
    /* TODO */
    constexpr std::size_t width = 4;

    constexpr float inv_m_f = 1.0f / static_cast<float>(std::numeric_limits<From>::max());
    constexpr float m_t = static_cast<float>(std::numeric_limits<To>::max());

    for(std::size_t i = 0; i < size; ++i)
    {
        to[i] = static_cast<To>(static_cast<float>(from[i]) * inv_m_f * m_t);
    }
}

template<typename From, typename To>
std::enable_if_t<is_integral_to_float_v<From, To>>
layer_convert_sse_kernel(const From* __restrict from,
                         To* __restrict to,
                         const std::size_t size) noexcept
{
    /* TODO */
    constexpr std::size_t width = 4;

    constexpr float inv_m = 1.0f / static_cast<float>(std::numeric_limits<From>::max());

    std::size_t i = 0;

    for(; i < size; ++i)
    {
        to[i] = static_cast<float>(from[i]) * inv_m;
    }
}

template<typename From, typename To>
std::enable_if_t<is_integral_to_half_v<From, To>>
layer_convert_sse_kernel(const From* __restrict from,
                         To* __restrict to,
                         const std::size_t size) noexcept
{
    /* TODO */
    constexpr float inv_m = 1.0f / static_cast<float>(std::numeric_limits<From>::max());

    for(std::size_t i = 0; i < size; ++i)
    {
        to[i] = static_cast<half>(static_cast<float>(from[i]) * inv_m);
    }
}

template<std::uint8_t from_depth>
struct SSEDispatcher
{
    template<std::uint8_t to_depth>
    static void dispatch_to(const void* __restrict from,
                            void* __restrict to, const std::size_t size) noexcept
    {
        using FromType = depth_to_type_t<from_depth>;
        using ToType = depth_to_type_t<to_depth>;

        if constexpr (std::is_same_v<FromType, ToType>)
        {
            return;
        }
        else
        {
            layer_convert_sse_kernel(static_cast<const FromType*>(from),
                                     static_cast<ToType*>(to),
                                     size);
        }
    }

    static void dispatch(const void* __restrict from,
                         void* __restrict to,
                         std::uint8_t to_depth,
                         std::size_t size) noexcept
    {
        switch(to_depth)
        {
            case LayerDepth_U8:
                SSEDispatcher::dispatch_to<LayerDepth_U8>(from, to, size);
                break;
            case LayerDepth_U16:
                SSEDispatcher::dispatch_to<LayerDepth_U16>(from, to, size);
                break;
            case LayerDepth_U32:
                SSEDispatcher::dispatch_to<LayerDepth_U32>(from, to, size);
                break;
            case LayerDepth_F16:
                SSEDispatcher::dispatch_to<LayerDepth_F16>(from, to, size);
                break;
            case LayerDepth_F32:
                SSEDispatcher::dispatch_to<LayerDepth_F32>(from, to, size);
                break;
        }
    }
};

void layer_convert_sse(const void* __restrict from,
                       void* __restrict to,
                       std::uint8_t from_depth,
                       std::uint8_t to_depth,
                       const std::size_t size) noexcept
{
    switch(from_depth)
    {
        case LayerDepth_U8:
            SSEDispatcher<LayerDepth_U8>::dispatch(from, to, to_depth, size);
            break;
        case LayerDepth_U16:
            SSEDispatcher<LayerDepth_U16>::dispatch(from, to, to_depth, size);
            break;
        case LayerDepth_U32:
            SSEDispatcher<LayerDepth_U32>::dispatch(from, to, to_depth, size);
            break;
        case LayerDepth_F16:
            SSEDispatcher<LayerDepth_F16>::dispatch(from, to, to_depth, size);
            break;
        case LayerDepth_F32:
            SSEDispatcher<LayerDepth_F32>::dispatch(from, to, to_depth, size);
            break;
    }
}

/******************************************/
/* AVX */
/******************************************/

template<typename From, typename To>
std::enable_if_t<std::is_same_v<From, To>>
layer_convert_avx_kernel(const From* __restrict from,
                         To* __restrict to,
                         const std::size_t size) noexcept
{
    LOV_UNUSED(from);
    LOV_UNUSED(to);
    LOV_UNUSED(size);
}

template<typename From, typename To>
std::enable_if_t<is_float_to_integral_v<From, To>>
layer_convert_avx_kernel(const From* __restrict from,
                         To* __restrict to,
                         const std::size_t size) noexcept
{
    constexpr std::size_t width = 8;

    const std::size_t simd_size = size - (size % width);

    constexpr float m = static_cast<float>(std::numeric_limits<To>::max());

    const __m256 m_256 = _mm256_set1_ps(m);
    const __m256 zeros = _mm256_setzero_ps();
    const __m256 ones = _mm256_set1_ps(1.0f);

    std::size_t i = 0;

    for(; i < simd_size; i += width)
    {
        __m256 f = _mm256_load_ps(std::addressof(from[i]));

        f = _mm256_max_ps(f, zeros);
        f = _mm256_min_ps(f, ones);
        f = _mm256_mul_ps(f, m_256);

        if constexpr (std::is_same_v<To, std::uint8_t>)
        {
            const __m256i i_values = _mm256_cvtps_epi32(f);
            const __m256i s_values = _mm256_packs_epi32(i_values, _mm256_permute2x128_si256(i_values, i_values, 1));
            const __m256i res = _mm256_packus_epi16(s_values, s_values);
            const __m128i res_128 = _mm256_castsi256_si128(res);

            _mm_storeu_si64(std::addressof(to[i]), res_128);
        }
        else if constexpr (std::is_same_v<To, std::uint16_t>)
        {
            const __m256i i_values = _mm256_cvtps_epi32(f);
            const __m256i res = _mm256_packus_epi32(i_values, _mm256_permute2x128_si256(i_values, i_values, 1));
            const __m128i res_128 = _mm256_castsi256_si128(res);

            _mm_store_si128(reinterpret_cast<__m128i*>(std::addressof(to[i])), res_128);
        }
        else if constexpr (std::is_same_v<To, std::uint32_t>)
        {
            /* No support for unsigned int 32 with avx2 */
            for(std::size_t j = 0; j < width; j++)
            {
                to[i + j] = static_cast<To>(std::clamp(from[i + j], 0.0f, 1.0f) * m);
            }
        }
        else
        {
            static_assert(0, "To integral type not supported");
        }
    }

    for(; i < size; ++i)
    {
        to[i] = static_cast<To>(std::clamp(from[i], 0.0f, 1.0f) * m);
    }
}

template<typename From, typename To>
std::enable_if_t<is_float_to_half_v<From, To>>
layer_convert_avx_kernel(const From* __restrict from,
                         To* __restrict to,
                         const std::size_t size) noexcept
{
    constexpr std::size_t width = 8;

    const std::size_t simd_size = size - (size % width);

    std::size_t i = 0;

    for(; i < simd_size; i += width)
    {
        const __m256 f = _mm256_load_ps(std::addressof(from[i]));
        const __m128i h = _mm256_cvtps_ph(f, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);

        _mm_store_si128(reinterpret_cast<__m128i*>(std::addressof(to[i])), h);
    }

    for(; i < size; ++i)
    {
        to[i] = static_cast<half>(from[i]);
    }
}

template<typename From, typename To>
std::enable_if_t<is_half_to_integral_v<From, To>>
layer_convert_avx_kernel(const From* __restrict from,
                         To* __restrict to,
                         const std::size_t size) noexcept
{
    constexpr std::size_t width = 8;

    const std::size_t simd_size = size - (size % width);

    constexpr float m = static_cast<float>(std::numeric_limits<To>::max());

    const __m256 m_256 = _mm256_set1_ps(m);
    const __m256 zeros = _mm256_setzero_ps();
    const __m256 ones = _mm256_set1_ps(1.0f);

    std::size_t i = 0;

    for(; i < simd_size; i += width)
    {
        const __m128i h = _mm_load_si128(reinterpret_cast<const __m128i*>(std::addressof(from[i])));
        __m256 f = _mm256_cvtph_ps(h);

        f = _mm256_max_ps(f, zeros);
        f = _mm256_min_ps(f, ones);
        f = _mm256_mul_ps(f, m_256);

        if constexpr (std::is_same_v<To, std::uint8_t>)
        {
            const __m256i i_values = _mm256_cvtps_epi32(f);
            const __m256i s_values = _mm256_packs_epi32(i_values, _mm256_permute2x128_si256(i_values, i_values, 1));
            const __m256i res = _mm256_packus_epi16(s_values, s_values);
            const __m128i res_128 = _mm256_castsi256_si128(res);

            _mm_storeu_si64(std::addressof(to[i]), res_128);
        }
        else if constexpr (std::is_same_v<To, std::uint16_t>)
        {
            const __m256i i_values = _mm256_cvtps_epi32(f);
            const __m256i res = _mm256_packus_epi32(i_values, _mm256_permute2x128_si256(i_values, i_values, 1));
            const __m128i res_128 = _mm256_castsi256_si128(res);

            _mm_store_si128(reinterpret_cast<__m128i*>(std::addressof(to[i])), res_128);
        }
        else if constexpr (std::is_same_v<To, std::uint32_t>)
        {
            /* No support for unsigned int 32 with avx2 */
            for(std::size_t j = 0; j < width; j++)
            {
                to[i + j] = static_cast<To>(std::clamp(static_cast<float>(from[i + j]),
                                                       0.0f,
                                                       1.0f) * m);
            }
        }
        else
        {
            static_assert(0, "To integral type not supported");
        }
    }

    for(; i < size; ++i)
    {
        to[i] = static_cast<To>(std::clamp(static_cast<float>(from[i]),
                                           0.0f,
                                           1.0f) * m);
    }
}

template<typename From, typename To>
std::enable_if_t<is_half_to_float_v<From, To>>
layer_convert_avx_kernel(const From* __restrict from,
                         To* __restrict to,
                         const std::size_t size) noexcept
{
    constexpr std::size_t width = 8;

    const std::size_t simd_size = size - (size % width);

    std::size_t i = 0;

    for(; i < simd_size; i += width)
    {
        const __m128i h = _mm_load_si128(reinterpret_cast<const __m128i*>(std::addressof(from[i])));
        const __m256 f = _mm256_cvtph_ps(h);

        _mm256_store_ps(std::addressof(to[i]), f);
    }

    for(; i < size; ++i)
    {
        to[i] = static_cast<float>(from[i]);
    }
}

template<typename From, typename To>
std::enable_if_t<is_integral_to_integral_v<From, To>>
layer_convert_avx_kernel(const From* __restrict from,
                         To* __restrict to,
                         const std::size_t size) noexcept
{
    constexpr std::size_t width = 8;

    const std::size_t simd_size = size - (size % width);

    constexpr float inv_m = 1.0f / static_cast<float>(std::numeric_limits<From>::max());
    constexpr float m = static_cast<float>(std::numeric_limits<To>::max());

    const __m256 inv_m_256 = _mm256_set1_ps(inv_m);
    const __m256 m_256 = _mm256_set1_ps(m);

    std::size_t i = 0;

    for(; i < simd_size; i += width)
    {
        __m256 f;

        if constexpr (std::is_same_v<From, std::uint8_t>)
        {
            const __m128i i_values = _mm_loadu_si64(std::addressof(from[i]));
            const __m256i s_values = _mm256_cvtepu8_epi32(i_values);
            f = _mm256_cvtepi32_ps(s_values);
        }
        else if constexpr (std::is_same_v<From, std::uint16_t>)
        {
            const __m128i i_values = _mm_loadu_si128(reinterpret_cast<const __m128i*>(std::addressof(from[i])));
            const __m256i s_values = _mm256_cvtepu16_epi32(i_values);
            f = _mm256_cvtepi32_ps(s_values);
        }
        else if constexpr (std::is_same_v<From, std::uint32_t>)
        {
            const __m256i i_values = _mm256_load_si256(reinterpret_cast<const __m256i*>(std::addressof(from[i])));
            f = stdromano::_mm256_cvtepu32_ps(i_values);
        }
        else
        {
            static_assert(0, "From integral type not supported");
        }

        f = _mm256_mul_ps(f, inv_m_256);
        f = _mm256_mul_ps(f, m_256);

        if constexpr (std::is_same_v<To, std::uint8_t>)
        {
            const __m256i i_values = _mm256_cvtps_epi32(f);
            const __m256i s_values = _mm256_packs_epi32(i_values, _mm256_permute2x128_si256(i_values, i_values, 1));
            const __m256i res = _mm256_packus_epi16(s_values, s_values);
            const __m128i res_128 = _mm256_castsi256_si128(res);

            _mm_storeu_si64(std::addressof(to[i]), res_128);
        }
        else if constexpr (std::is_same_v<To, std::uint16_t>)
        {
            const __m256i i_values = _mm256_cvtps_epi32(f);
            const __m256i res = _mm256_packus_epi32(i_values, _mm256_permute2x128_si256(i_values, i_values, 1));
            const __m128i res_128 = _mm256_castsi256_si128(res);

            _mm_store_si128(reinterpret_cast<__m128i*>(std::addressof(to[i])), res_128);
        }
        else if constexpr (std::is_same_v<To, std::uint32_t>)
        {
            /* No support for unsigned int 32 with avx */
            for(std::size_t j = 0; j < width; j++)
            {
                to[i + j] = static_cast<To>(static_cast<float>(from[i + j]) * inv_m * m);
            }
        }
        else
        {
            static_assert(0, "To integral type not supported");
        }
    }

    for(; i < size; ++i)
    {
        to[i] = static_cast<To>(static_cast<float>(from[i]) * inv_m * m);
    }
}

template<typename From, typename To>
std::enable_if_t<is_integral_to_float_v<From, To>>
layer_convert_avx_kernel(const From* __restrict from,
                         To* __restrict to,
                         const std::size_t size) noexcept
{
    constexpr std::size_t width = 8;

    const std::size_t simd_size = size - (size % width);

    constexpr float inv_m = 1.0f / static_cast<float>(std::numeric_limits<From>::max());

    const __m256 inv_m_256 = _mm256_set1_ps(inv_m);

    std::size_t i = 0;

    for(; i < simd_size; i += width)
    {
        __m256 f;

        if constexpr (std::is_same_v<From, std::uint8_t>)
        {
            const __m128i i_values = _mm_loadu_si64(std::addressof(from[i]));
            const __m256i s_values = _mm256_cvtepu8_epi32(i_values);
            f = _mm256_cvtepi32_ps(s_values);
        }
        else if constexpr (std::is_same_v<From, std::uint16_t>)
        {
            const __m128i i_values = _mm_loadu_si128(reinterpret_cast<const __m128i*>(std::addressof(from[i])));
            const __m256i s_values = _mm256_cvtepu16_epi32(i_values);
            f = _mm256_cvtepi32_ps(s_values);
        }
        else if constexpr (std::is_same_v<From, std::uint32_t>)
        {
            const __m256i i_values = _mm256_load_si256(reinterpret_cast<const __m256i*>(std::addressof(from[i])));
            f = stdromano::_mm256_cvtepu32_ps(i_values);
        }
        else
        {
            static_assert(0, "From integral type not supported");
        }

        f = _mm256_mul_ps(f, inv_m_256);

        _mm256_store_ps(std::addressof(to[i]), f);
    }

    for(; i < size; ++i)
    {
        to[i] = static_cast<float>(from[i]) * inv_m;
    }
}

template<typename From, typename To>
std::enable_if_t<is_integral_to_half_v<From, To>>
layer_convert_avx_kernel(const From* __restrict from,
                         To* __restrict to,
                         const std::size_t size) noexcept
{
    constexpr std::size_t width = 8;

    const std::size_t simd_size = size - (size % width);

    constexpr float inv_m = 1.0f / static_cast<float>(std::numeric_limits<From>::max());

    const __m256 inv_m_256 = _mm256_set1_ps(inv_m);

    std::size_t i = 0;

    for(; i < simd_size; i += width)
    {
        __m256 f;

        if constexpr (std::is_same_v<From, std::uint8_t>)
        {
            const __m128i i_values = _mm_loadu_si64(std::addressof(from[i]));
            const __m256i s_values = _mm256_cvtepu8_epi32(i_values);
            f = _mm256_cvtepi32_ps(s_values);
        }
        else if constexpr (std::is_same_v<From, std::uint16_t>)
        {
            const __m128i i_values = _mm_loadu_si128(reinterpret_cast<const __m128i*>(std::addressof(from[i])));
            const __m256i s_values = _mm256_cvtepu16_epi32(i_values);
            f = _mm256_cvtepi32_ps(s_values);
        }
        else if constexpr (std::is_same_v<From, std::uint32_t>)
        {
            const __m256i i_values = _mm256_load_si256(reinterpret_cast<const __m256i*>(std::addressof(from[i])));
            f = stdromano::_mm256_cvtepu32_ps(i_values);
        }
        else
        {
            static_assert(0, "From integral type not supported");
        }

        f = _mm256_mul_ps(f, inv_m_256);
        const __m128i h = _mm256_cvtps_ph(f, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);

        _mm_store_si128(reinterpret_cast<__m128i*>(std::addressof(to[i])), h);
    }

    for(; i < size; ++i)
    {
        to[i] = static_cast<half>(static_cast<float>(from[i]) * inv_m);
    }
}

template<std::uint8_t from_depth>
struct AVXDispatcher
{
    template<std::uint8_t to_depth>
    static void dispatch_to(const void* __restrict from,
                            void* __restrict to, const std::size_t size) noexcept
    {
        using FromType = depth_to_type_t<from_depth>;
        using ToType = depth_to_type_t<to_depth>;

        if constexpr (std::is_same_v<FromType, ToType>)
        {
            return;
        }
        else
        {
            layer_convert_avx_kernel(static_cast<const FromType*>(from),
                                     static_cast<ToType*>(to),
                                     size);
        }
    }

    static void dispatch(const void* __restrict from,
                         void* __restrict to,
                         std::uint8_t to_depth,
                         std::size_t size) noexcept
    {
        switch(to_depth)
        {
            case LayerDepth_U8:
                AVXDispatcher::dispatch_to<LayerDepth_U8>(from, to, size);
                break;
            case LayerDepth_U16:
                AVXDispatcher::dispatch_to<LayerDepth_U16>(from, to, size);
                break;
            case LayerDepth_U32:
                AVXDispatcher::dispatch_to<LayerDepth_U32>(from, to, size);
                break;
            case LayerDepth_F16:
                AVXDispatcher::dispatch_to<LayerDepth_F16>(from, to, size);
                break;
            case LayerDepth_F32:
                AVXDispatcher::dispatch_to<LayerDepth_F32>(from, to, size);
                break;
        }
    }
};

void layer_convert_avx(const void* __restrict from,
                       void* __restrict to,
                       std::uint8_t from_depth,
                       std::uint8_t to_depth,
                       const std::size_t size) noexcept
{
    switch(from_depth)
    {
        case LayerDepth_U8:
            AVXDispatcher<LayerDepth_U8>::dispatch(from, to, to_depth, size);
            break;
        case LayerDepth_U16:
            AVXDispatcher<LayerDepth_U16>::dispatch(from, to, to_depth, size);
            break;
        case LayerDepth_U32:
            AVXDispatcher<LayerDepth_U32>::dispatch(from, to, to_depth, size);
            break;
        case LayerDepth_F16:
            AVXDispatcher<LayerDepth_F16>::dispatch(from, to, to_depth, size);
            break;
        case LayerDepth_F32:
            AVXDispatcher<LayerDepth_F32>::dispatch(from, to, to_depth, size);
            break;
    }
}

/******************************************/
/* AVX2 */
/******************************************/

template<typename From, typename To>
std::enable_if_t<std::is_same_v<From, To>>
layer_convert_avx2_kernel(const From* __restrict from,
                          To* __restrict to,
                          const std::size_t size) noexcept
{
    LOV_UNUSED(from);
    LOV_UNUSED(to);
    LOV_UNUSED(size);
}

template<typename From, typename To>
std::enable_if_t<is_float_to_integral_v<From, To>>
layer_convert_avx2_kernel(const From* __restrict from,
                          To* __restrict to,
                          const std::size_t size) noexcept
{
    layer_convert_avx_kernel(from, to, size);
}

template<typename From, typename To>
std::enable_if_t<is_float_to_half_v<From, To>>
layer_convert_avx2_kernel(const From* __restrict from,
                         To* __restrict to,
                         const std::size_t size) noexcept
{
    layer_convert_avx_kernel(from, to, size);
}

template<typename From, typename To>
std::enable_if_t<is_half_to_integral_v<From, To>>
layer_convert_avx2_kernel(const From* __restrict from,
                         To* __restrict to,
                         const std::size_t size) noexcept
{
    layer_convert_avx_kernel(from, to, size);
}

template<typename From, typename To>
std::enable_if_t<is_half_to_float_v<From, To>>
layer_convert_avx2_kernel(const From* __restrict from,
                          To* __restrict to,
                          const std::size_t size) noexcept
{
    layer_convert_avx_kernel(from, to, size);
}

template<typename From, typename To>
std::enable_if_t<is_integral_to_integral_v<From, To>>
layer_convert_avx2_kernel(const From* __restrict from,
                          To* __restrict to,
                          const std::size_t size) noexcept
{
    layer_convert_avx_kernel(from, to, size);
}

template<typename From, typename To>
std::enable_if_t<is_integral_to_float_v<From, To>>
layer_convert_avx2_kernel(const From* __restrict from,
                          To* __restrict to,
                          const std::size_t size) noexcept
{
    layer_convert_avx_kernel(from, to, size);
}

template<typename From, typename To>
std::enable_if_t<is_integral_to_half_v<From, To>>
layer_convert_avx2_kernel(const From* __restrict from,
                          To* __restrict to,
                          const std::size_t size) noexcept
{
    layer_convert_avx_kernel(from, to, size);
}

template<std::uint8_t from_depth>
struct AVX2Dispatcher
{
    template<std::uint8_t to_depth>
    static void dispatch_to(const void* __restrict from,
                            void* __restrict to, const std::size_t size) noexcept
    {
        using FromType = depth_to_type_t<from_depth>;
        using ToType = depth_to_type_t<to_depth>;

        if constexpr (std::is_same_v<FromType, ToType>)
        {
            return;
        }
        else
        {
            layer_convert_avx2_kernel(static_cast<const FromType*>(from),
                                     static_cast<ToType*>(to),
                                     size);
        }
    }

    static void dispatch(const void* __restrict from,
                         void* __restrict to,
                         std::uint8_t to_depth,
                         std::size_t size) noexcept
    {
        switch(to_depth)
        {
            case LayerDepth_U8:
                AVX2Dispatcher::dispatch_to<LayerDepth_U8>(from, to, size);
                break;
            case LayerDepth_U16:
                AVX2Dispatcher::dispatch_to<LayerDepth_U16>(from, to, size);
                break;
            case LayerDepth_U32:
                AVX2Dispatcher::dispatch_to<LayerDepth_U32>(from, to, size);
                break;
            case LayerDepth_F16:
                AVX2Dispatcher::dispatch_to<LayerDepth_F16>(from, to, size);
                break;
            case LayerDepth_F32:
                AVX2Dispatcher::dispatch_to<LayerDepth_F32>(from, to, size);
                break;
        }
    }
};

void layer_convert_avx2(const void* __restrict from,
                       void* __restrict to,
                       std::uint8_t from_depth,
                       std::uint8_t to_depth,
                       const std::size_t size) noexcept
{
    switch(from_depth)
    {
        case LayerDepth_U8:
            AVX2Dispatcher<LayerDepth_U8>::dispatch(from, to, to_depth, size);
            break;
        case LayerDepth_U16:
            AVX2Dispatcher<LayerDepth_U16>::dispatch(from, to, to_depth, size);
            break;
        case LayerDepth_U32:
            AVX2Dispatcher<LayerDepth_U32>::dispatch(from, to, to_depth, size);
            break;
        case LayerDepth_F16:
            AVX2Dispatcher<LayerDepth_F16>::dispatch(from, to, to_depth, size);
            break;
        case LayerDepth_F32:
            AVX2Dispatcher<LayerDepth_F32>::dispatch(from, to, to_depth, size);
            break;
    }
}

/******************************************/
/* Dispatcher */
/******************************************/

void Layer::convert(const std::uint8_t new_depth) noexcept
{
    if(new_depth == this->_depth)
    {
        return;
    }

    LOV_ASSERT(this->_data != nullptr, "Layer has not data loaded (data is nullptr)");

    void* new_data = stdromano::mem_aligned_alloc(this->nelements() * layer_depth_as_byte_size(new_depth),
                                                  Layer::ALIGNMENT);

    switch(stdromano::simd_get_vectorization_mode())
    {
        case stdromano::VectorizationMode_Scalar:
            layer_convert_scalar(this->_data, new_data, this->_depth, new_depth, this->nelements());
            break;
        case stdromano::VectorizationMode_SSE:
            layer_convert_sse(this->_data, new_data, this->_depth, new_depth, this->nelements());
            break;
        case stdromano::VectorizationMode_AVX:
            layer_convert_avx(this->_data, new_data, this->_depth, new_depth, this->nelements());
            break;
        case stdromano::VectorizationMode_AVX2:
            layer_convert_avx2(this->_data, new_data, this->_depth, new_depth, this->nelements());
            break;
        default:
            layer_convert_scalar(this->_data, new_data, this->_depth, new_depth, this->nelements());
            break;
    }

    stdromano::mem_aligned_free(this->_data);

    this->_data = new_data;
    this->_depth = new_depth;
}

LOV_NAMESPACE_END
