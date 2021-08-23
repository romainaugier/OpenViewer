// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#pragma once

#include "implot.h"
#include "utils/memory/alloc.h"

#include <stdint.h>
#include <algorithm>
#include <immintrin.h>


struct Plot
{
	std::vector<double> bin_counts;
	uint64_t image_size = 0;
	float* r_array = nullptr;
	float* g_array = nullptr;
	float* b_array = nullptr;
	float* pos_array = nullptr;
	uint16_t image_width = 0;
	uint16_t image_height = 0;
	uint16_t bin_x = 500;
	uint16_t bin_y = 250;
	uint8_t mipmap_idx = 0;
	unsigned int has_been_initialized : 1;

	Plot()
	{
		has_been_initialized = 0;
		bin_counts.resize(bin_x * bin_y);
		
		for (int b = 0; b < (bin_x * bin_y); b += 4)
		{
			const __m256d tmp = _mm256_set1_pd(0.0);
			_mm256_storeu_pd(&bin_counts[b], tmp);
		}
	}

	void Initialize(uint64_t size, uint16_t width, uint16_t height) noexcept;
	void Update(float* image) noexcept;
	void Release() noexcept;

	void Waveform() const noexcept;
	void Vectorscope() const noexcept;
	void Parade() const noexcept;
	void Histogram() const noexcept;
};
