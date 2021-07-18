// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include <chrono>

#include "decl.h"

struct Profiler
{
	float avg_ocio_transform_time = 0.0f;
	float avg_unpack_calc_time = 0.0f;
	float avg_plot_time = 0.0f;
	float avg_plot_draw_time = 0.0f;
	float avg_load_time = 0.0f;
	float avg_frame_time = 0.0f;

	// to check memory usage at runtime
	float current_memory_usage = 0;
	float loader_size = 0;
	float display_size = 0;
	float ocio_size = 0;

	OPENVIEWER_FORCEINLINE auto Start() const noexcept
	{
		return std::chrono::system_clock::now();
	}

	OPENVIEWER_FORCEINLINE auto End() const noexcept
	{
		return std::chrono::system_clock::now();
	}

	OPENVIEWER_FORCEINLINE void Ocio(std::chrono::time_point<std::chrono::system_clock>& start, 
			  std::chrono::time_point<std::chrono::system_clock>& end) noexcept
	{
		avg_ocio_transform_time = (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() + avg_ocio_transform_time) / 2.0f;
	}

	OPENVIEWER_FORCEINLINE void Unpack(std::chrono::time_point<std::chrono::system_clock>& start,
		std::chrono::time_point<std::chrono::system_clock>& end) noexcept
	{
		avg_unpack_calc_time = (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() + avg_unpack_calc_time) / 2.0f;
	}

	OPENVIEWER_FORCEINLINE void Plot(std::chrono::time_point<std::chrono::system_clock>& start,
		std::chrono::time_point<std::chrono::system_clock>& end) noexcept
	{
		avg_plot_time = (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() + avg_plot_time) / 2.0f;
	}

	OPENVIEWER_FORCEINLINE void DrawPlot(std::chrono::time_point<std::chrono::system_clock>& start,
		std::chrono::time_point<std::chrono::system_clock>& end) noexcept
	{
		avg_plot_draw_time = (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() + avg_plot_draw_time) / 2.0f;
	}

	 OPENVIEWER_FORCEINLINE void Load(std::chrono::time_point<std::chrono::system_clock>& start,
		std::chrono::time_point<std::chrono::system_clock>& end) noexcept
	{
		avg_load_time = (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() + avg_load_time) / 2.0f;
	}

	 OPENVIEWER_FORCEINLINE void Frame(std::chrono::time_point<std::chrono::system_clock>& start,
		 std::chrono::time_point<std::chrono::system_clock>& end) noexcept
	 {
		 avg_frame_time = (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() + avg_frame_time) / 2.0f;
	 }
};