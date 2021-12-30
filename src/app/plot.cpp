// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#include "plot.h"
#undef min


void Plot::Initialize(uint64_t size, uint16_t width, uint16_t height) noexcept
{
	has_been_initialized = 1;

	mipmap_idx = floor(height / 1000);

	if(mipmap_idx == 0) image_size = size;
	else image_size = size / (mipmap_idx * 4);
	
	image_width = width;
	image_height = height;

	r_array = static_cast<float*>(OvAlloc((image_size / 4) * sizeof(float), 32));
	g_array = static_cast<float*>(OvAlloc((image_size / 4) * sizeof(float), 32));
	b_array = static_cast<float*>(OvAlloc((image_size / 4) * sizeof(float), 32));
	pos_array = static_cast<float*>(OvAlloc((image_size / 4) * sizeof(float), 32));
}

void Plot::Update(float* image) noexcept
{
	// uint64_t idx = 0;

	// const __m256 luma_coeff_r = _mm256_set1_ps(0.2126f);
	// const __m256 luma_coeff_g = _mm256_set1_ps(0.7152f);
	// const __m256 luma_coeff_b = _mm256_set1_ps(0.0722f);
	// const __m256 max = _mm256_set1_ps(1.0f);
	// const __m256 min = _mm256_setzero_ps();

	// for (uint64_t i = 0; i < image_size; i += 32)
	// {
	// 	__m256 r = _mm256_set_ps(image[i + 0], image[i + 4], image[i + 8], image[i + 12],
	// 		image[i + 16], image[i + 20], image[i + 24], image[i + 28]);

	// 	__m256 g = _mm256_set_ps(image[i + 1], image[i + 5], image[i + 9], image[i + 13],
	// 		image[i + 17], image[i + 21], image[i + 25], image[i + 29]);

	// 	__m256 b = _mm256_set_ps(image[i + 2], image[i + 6], image[i + 10], image[i + 14],
	// 		image[i + 18], image[i + 22], image[i + 26], image[i + 30]);

	// 	__m256 mask = _mm256_cmp_ps(r, max, 14);
	// 	r = _mm256_blendv_ps(r, max, mask);

	// 	mask = _mm256_cmp_ps(g, max, 14);
	// 	g = _mm256_blendv_ps(g, max, mask);

	// 	mask = _mm256_cmp_ps(b, max, 14);
	// 	b = _mm256_blendv_ps(b, max, mask);
		
	// 	__m256i indices = _mm256_set_epi32(i + 0, i + 4, i + 8, i + 12, i + 16, i + 20, i + 24, i + 28);
	// 	//__m256i multiplier = _mm256_set1_epi32(2);
	// 	__m256i divisors = _mm256_set1_epi32(mipmap_idx);
	// 	__m256i positions_int;
	// 	//indices = _mm256_mul_epi32(indices, multiplier);
	// 	//indices = _mm256_div_epi32(indices, divisors);
	// 	divisors = _mm256_set1_epi32(image_width);
	// 	indices = _mm256_divrem_epi32(&positions_int, indices, divisors);
	// 	__m256 positions = _mm256_cvtepi32_ps(positions_int);


	// 	_mm256_store_ps(&r_array[idx], r);
	// 	_mm256_store_ps(&g_array[idx], g);
	// 	_mm256_store_ps(&b_array[idx], b);
	// 	_mm256_store_ps(&pos_array[idx], positions);

	// 	idx += 8;
	// }
	
}

void Plot::Release() noexcept
{
	OvFree(static_cast<void*>(r_array));
	OvFree(static_cast<void*>(g_array));
	OvFree(static_cast<void*>(b_array));
	OvFree(static_cast<void*>(pos_array));

	for (int b = 0; b < (bin_x * bin_y); b += 4)
	{
		const __m256d tmp = _mm256_set1_pd(0.0);
		_mm256_storeu_pd(&bin_counts[b], tmp);
	}

	r_array = nullptr;
	g_array = nullptr;
	b_array = nullptr;
	pos_array = nullptr;

	has_been_initialized = 0;
}

void Plot::Parade() const noexcept
{
	if (has_been_initialized > 0)
	{
		ImGui::Begin("Parade");
		{
			ImGui::SmallButton("R");
			ImGui::SameLine();
			ImGui::SmallButton("G");
			ImGui::SameLine();
			ImGui::SmallButton("B");

			

			if (ImPlot::BeginPlot("R", nullptr, nullptr, ImVec2(-1, 0), ImPlotFlags_CanvasOnly, ImPlotAxisFlags_AutoFit))
			{
				ImPlot::PushColormap("RedWF");
				ImPlot::PlotHistogram2D("", pos_array, r_array, (image_size / 4), bin_counts, bin_x, bin_y, false, ImPlotLimits(0, image_width, 0, 1.05), false);
				ImPlot::PopColormap();
				ImPlot::EndPlot();
			}
			if (ImPlot::BeginPlot("G", nullptr, nullptr, ImVec2(-1, 0), ImPlotFlags_CanvasOnly, ImPlotAxisFlags_AutoFit))
			{
				ImPlot::PushColormap("GreenWF");
				ImPlot::PlotHistogram2D("", pos_array, g_array, (image_size / 4), bin_counts, bin_x, bin_y, false, ImPlotLimits(0, image_width, 0, 1.05), false);
				ImPlot::PopColormap();
				ImPlot::EndPlot();
			}
			if (ImPlot::BeginPlot("B", nullptr, nullptr, ImVec2(-1, 0), ImPlotFlags_CanvasOnly, ImPlotAxisFlags_AutoFit))
			{																	  
				ImPlot::PushColormap("BlueWF");									  
				ImPlot::PlotHistogram2D("", pos_array, b_array, (image_size / 4), bin_counts, bin_x, bin_y, false, ImPlotLimits(0, image_width, 0, 1.05), false);
				ImPlot::PopColormap();
				ImPlot::EndPlot();
			}
			
		}
		ImGui::End();
	}
}

void Plot::Vectorscope() const noexcept
{

}

void Plot::Waveform() const noexcept
{

}

void Plot::Histogram() const noexcept
{

}