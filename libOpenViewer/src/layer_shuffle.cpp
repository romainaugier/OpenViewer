// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/image.hpp"

#include "stdromano/logger.hpp"
#include "stdromano/simd.hpp"
#include "stdromano/bits.hpp"

LOV_NAMESPACE_BEGIN

/*
 * TODO: do a mini jit-compiler for shuffle operations instead of this, will be much much more efficient
 */

/* Layer shuffle */

enum ShuffleChannel : std::uint8_t
{
    ShuffleChannel_NoChannel = 0x0,

    ShuffleChannel_R = 0x1,
    ShuffleChannel_G = 0x2,
    ShuffleChannel_B = 0x4,
    ShuffleChannel_A = 0x8,

    ShuffleChannel_0 = 0x10,
    ShuffleChannel_1 = 0x20,

    ShuffleChannel_AvgLum = 0x40,
    ShuffleChannel_WeightedLum = 0x80,
};

static const std::uint32_t ShuffleR = 0x01010101;
static const std::uint32_t ShuffleG = 0x02020202;
static const std::uint32_t ShuffleB = 0x04040404;
static const std::uint32_t ShuffleA = 0x08080808;

static const std::uint32_t Shuffle0 = 0x10101010;
static const std::uint32_t Shuffle1 = 0x20202020;

static const std::uint32_t ShuffleAvgLum = 0x40404040;
static const std::uint32_t ShuffleWeightedLum = 0x80808080;

std::tuple<std::int32_t, std::size_t> build_shuffle_mask(const stdromano::StringD& mask,
                                                         std::uint32_t nchannels_input) noexcept
{
    if(mask.size() > 4)
    {
        stdromano::log_error("Invalid shuffle mask: {} (Mask is too long)", mask);
        return std::make_tuple(0, 0);
    }

    std::int32_t bit_mask = 0;
    std::size_t position = 0;

    for(const auto& c : mask)
    {
        std::uint8_t channel_val = 0;

        switch(c)
        {
            case 'R':
            case 'r':
                channel_val = ShuffleChannel_R;
                break;

            case 'G':
            case 'g':
                channel_val = nchannels_input < 2 ? ShuffleChannel_0 : ShuffleChannel_G;
                break;

            case 'B':
            case 'b':
                channel_val = nchannels_input < 3 ? ShuffleChannel_0 : ShuffleChannel_B;
                break;

            case 'A':
            case 'a':
                channel_val = nchannels_input < 4 ? ShuffleChannel_1 : ShuffleChannel_A;
                break;

            case '0':
                channel_val = ShuffleChannel_0;
                break;

            case '1':
                channel_val = ShuffleChannel_1;
                break;

            case 'l':
                channel_val = ShuffleChannel_WeightedLum;
                break;

            case 'L':
                channel_val = ShuffleChannel_AvgLum;
                break;

            default:
                stdromano::log_error("Invalid shuffle mask: {} (Unknown channel {})", mask, c);
                return std::make_tuple(0, 0);
        }

        bit_mask |= static_cast<std::uint32_t>(channel_val) << (8 * position);

        position++;
    }

    return std::make_tuple(bit_mask, position);
}

/* Scalar shuffle */

template<typename T>
LOV_FORCE_INLINE T get_zero_value() noexcept { return static_cast<T>(0); }

template<typename T>
LOV_FORCE_INLINE T get_one_value() noexcept
{
    if constexpr (std::is_same_v<T, float> || std::is_same_v<T, half>)
    {
        return static_cast<T>(1.0);
    }
    else
    {
        return std::numeric_limits<T>::max();
    }
}

template<typename T>
LOV_FORCE_INLINE T compute_avg_luminance(const T* pixel) noexcept
{
    if constexpr (std::is_same_v<T, float> || std::is_same_v<T, half>)
    {
        return (pixel[0] + pixel[1] + pixel[2]) * T(0.33333333);
    }
    else
    {
        constexpr float inv_max = 1.0f / static_cast<float>(std::numeric_limits<T>::max());

        const float r = static_cast<float>(pixel[0]) * inv_max;
        const float g = static_cast<float>(pixel[1]) * inv_max;
        const float b = static_cast<float>(pixel[2]) * inv_max;

        const float lum = (r + g + b) * 0.33333333f;

        return static_cast<T>(lum * static_cast<float>(std::numeric_limits<T>::max()));
    }
}

template<typename T>
LOV_FORCE_INLINE T compute_weighted_luminance(const T* pixel) noexcept
{
    if constexpr (std::is_same_v<T, float> || std::is_same_v<T, half>)
    {
        return pixel[0] * T(0.2126) + pixel[1] * T(0.7152) + pixel[2] * T(0.0722);
    }
    else
    {
        constexpr float inv_max = 1.0f / static_cast<float>(std::numeric_limits<T>::max());

        const float r = static_cast<float>(pixel[0]) * inv_max;
        const float g = static_cast<float>(pixel[1]) * inv_max;
        const float b = static_cast<float>(pixel[2]) * inv_max;

        const float lum = r * 0.2126f + g * 0.7152f + b * 0.0722f;

        return static_cast<T>(lum * static_cast<float>(std::numeric_limits<T>::max()));
    }
}

template<typename T>
void shuffle_scalar_kernel(const T* __restrict from,
                           T* __restrict to,
                           std::uint32_t mask,
                           std::int32_t width,
                           std::int32_t height,
                           std::uint8_t nchannels,
                           std::uint8_t output_channels)
{
    const std::size_t total_pixels = static_cast<std::size_t>(width) *
                                     static_cast<std::size_t>(height);

    std::uint8_t channels[4];

    for(int i = 0; i < 4; ++i)
    {
        channels[i] = static_cast<std::uint8_t>((mask >> (i * 8)) & 0xFF);
    }

    for(std::size_t pixel = 0; pixel < total_pixels; ++pixel)
    {
        const T* src_pixel = from + pixel * nchannels;
        T* dst_pixel = to + pixel * output_channels;

        for(std::uint8_t out_ch = 0; out_ch < output_channels; ++out_ch)
        {
            const std::uint8_t channel_op = channels[out_ch];

            switch(channel_op)
            {
                case ShuffleChannel_R:
                case ShuffleChannel_G:
                case ShuffleChannel_B:
                case ShuffleChannel_A:
                {
                    const std::uint8_t src_ch = static_cast<std::uint8_t>(stdromano::ctz_u64(static_cast<std::uint64_t>(channel_op)));
                    dst_pixel[out_ch] = static_cast<T>((src_ch < nchannels) ? src_pixel[src_ch] :
                                                                              get_zero_value<T>());
                    break;
                }

                case ShuffleChannel_0:
                {
                    dst_pixel[out_ch] = get_zero_value<T>();
                    break;
                }

                case ShuffleChannel_1:
                {
                    dst_pixel[out_ch] = get_one_value<T>();
                    break;
                }

                case ShuffleChannel_AvgLum:
                {
                    dst_pixel[out_ch] = compute_avg_luminance(src_pixel);
                    break;
                }

                case ShuffleChannel_WeightedLum:
                {
                    dst_pixel[out_ch] = compute_weighted_luminance(src_pixel);
                    break;
                }

                default:
                {
                    dst_pixel[out_ch] = get_zero_value<T>();
                    break;
                }
            }
        }
    }
}

void shuffle_scalar(const void* __restrict from,
                    void* __restrict to,
                    std::uint32_t mask,
                    std::int32_t width,
                    std::int32_t height,
                    std::uint8_t nchannels,
                    std::uint8_t depth,
                    std::uint8_t output_channels = 4) noexcept
{
    switch(depth)
    {
        case LayerDepth_U8:
            shuffle_scalar_kernel(static_cast<const std::uint8_t*>(from),
                         static_cast<std::uint8_t*>(to),
                         mask, width, height, nchannels, output_channels);
            break;
        case LayerDepth_U16:
            shuffle_scalar_kernel(static_cast<const std::uint16_t*>(from),
                         static_cast<std::uint16_t*>(to),
                         mask, width, height, nchannels, output_channels);
            break;
        case LayerDepth_U32:
            shuffle_scalar_kernel(static_cast<const std::uint32_t*>(from),
                         static_cast<std::uint32_t*>(to),
                         mask, width, height, nchannels, output_channels);
            break;
        case LayerDepth_F16:
            shuffle_scalar_kernel(static_cast<const half*>(from),
                         static_cast<half*>(to),
                         mask, width, height, nchannels, output_channels);
            break;
        case LayerDepth_F32:
            shuffle_scalar_kernel(static_cast<const float*>(from),
                         static_cast<float*>(to),
                         mask, width, height, nchannels, output_channels);
            break;
    }
}

/* SSE Shuffle */

void shuffle_sse(const void* __restrict from,
                 void* __restrict to,
                 std::uint32_t mask,
                 std::int32_t width,
                 std::int32_t height,
                 std::uint8_t nchannels,
                 std::uint8_t depth,
                 std::uint8_t output_channels = 4) noexcept
{
    const char* _from = static_cast<const char*>(from);
    char* _to = static_cast<char*>(to);


}

/* AVX Shuffle */

void shuffle_avx(const void* __restrict from,
                 void* __restrict to,
                 std::uint32_t mask,
                 std::int32_t width,
                 std::int32_t height,
                 std::uint8_t nchannels,
                 std::uint8_t depth,
                 std::uint8_t output_channels = 4)
{
    const char* _from = static_cast<const char*>(from);
    char* _to = static_cast<char*>(to);


}

/* AVX2 Shuffle */

void shuffle_avx2(const void* __restrict from,
                  void* __restrict to,
                  std::uint32_t mask,
                  std::int32_t width,
                  std::int32_t height,
                  std::uint8_t nchannels,
                  std::uint8_t depth,
                  std::uint8_t output_channels = 4)
{
    const char* _from = static_cast<const char*>(from);
    char* _to = static_cast<char*>(to);


}

/* Dispatcher */

void Layer::shuffle(const stdromano::StringD& mask) noexcept
{
    auto [bit_mask, mask_size] = build_shuffle_mask(mask, this->_nchannels);

    if(bit_mask == 0)
    {
        return;
    }

    LOV_ASSERT(this->_data != nullptr, "Layer has not data loaded (data is nullptr)");

    const std::size_t new_data_size = this->_parent->get_data_width() *
                                      this->_parent->get_data_height() *
                                      layer_depth_as_byte_size(this->_depth) *
                                      mask_size;

    void* new_data = stdromano::mem_aligned_alloc(new_data_size, Layer::ALIGNMENT);

#if 1
    shuffle_scalar(this->_data,
                   new_data,
                   bit_mask,
                   this->_parent->get_data_width(),
                   this->_parent->get_data_height(),
                   this->_nchannels,
                   this->_depth,
                   static_cast<std::uint8_t>(mask_size));
#else
    switch(stdromano::simd_get_vectorization_mode())
    {
        case stdromano::VectorizationMode_Scalar:
            shuffle_scalar(this->_data,
                           new_data,
                           bit_mask,
                           this->_parent->get_data_width(),
                           this->_parent->get_data_height(),
                           this->_nchannels,
                           this->_depth,
                           static_cast<std::uint8_t>(mask_size));
            break;
        case stdromano::VectorizationMode_SSE:
            shuffle_sse(this->_data,
                        new_data,
                        bit_mask,
                        this->_parent->get_data_width(),
                        this->_parent->get_data_height(),
                        this->_nchannels,
                        this->_depth,
                        static_cast<std::uint8_t>(mask_size));
            break;
        case stdromano::VectorizationMode_AVX:
            shuffle_avx(this->_data,
                        new_data,
                        bit_mask,
                        this->_parent->get_data_width(),
                        this->_parent->get_data_height(),
                        this->_nchannels,
                        this->_depth,
                        static_cast<std::uint8_t>(mask_size));
            break;
        case stdromano::VectorizationMode_AVX2:
            shuffle_avx2(this->_data,
                         new_data,
                         bit_mask,
                         this->_parent->get_data_width(),
                         this->_parent->get_data_height(),
                         this->_nchannels,
                         this->_depth,
                         static_cast<std::uint8_t>(mask_size));
            break;
        default:
            break;
    }
#endif

    stdromano::mem_aligned_free(this->_data);
    this->_data = new_data;
    this->_nchannels = mask_size;
}


LOV_NAMESPACE_END
