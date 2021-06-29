// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include <chrono>

struct Profiler
{
	float avg_ocio_transform_time = 0.0f;
	float avg_plot_calc_time = 0.0f;
	float avg_load_time = 0.0f;
	float avg_frame_time = 0.0f;

	__forceinline auto Start() const noexcept
	{
		return std::chrono::system_clock::now();
	}

	__forceinline auto End() const noexcept
	{
		return std::chrono::system_clock::now();
	}

	__forceinline void Ocio(std::chrono::time_point<std::chrono::system_clock>& start, 
			  std::chrono::time_point<std::chrono::system_clock>& end) noexcept
	{
		avg_ocio_transform_time = (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() + avg_ocio_transform_time) / 2.0f;
	}

	__forceinline void Plot(std::chrono::time_point<std::chrono::system_clock>& start,
		std::chrono::time_point<std::chrono::system_clock>& end) noexcept
	{
		avg_plot_calc_time = (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() + avg_plot_calc_time) / 2.0f;
	}

	 __forceinline void Load(std::chrono::time_point<std::chrono::system_clock>& start,
		std::chrono::time_point<std::chrono::system_clock>& end) noexcept
	{
		avg_load_time = (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() + avg_load_time) / 2.0f;
	}

	 __forceinline void Frame(std::chrono::time_point<std::chrono::system_clock>& start,
		 std::chrono::time_point<std::chrono::system_clock>& end) noexcept
	 {
		 avg_frame_time = (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() + avg_frame_time) / 2.0f;
	 }
};