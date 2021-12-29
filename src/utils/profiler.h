// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include <chrono>
#include <unordered_map>
#include <string>

#include "decl.h"

// Very simple profiler to get timings of some actions at runtime to help finding hotspots in the application

// There is also a simple map to get the memory usage of different structures used by OpenViewer, to help
// see what is consuming more or less memory

struct Profiler
{
	std::unordered_map<std::string, float> times;
	std::unordered_map<std::string, float> mem_usage;

	OPENVIEWER_FORCEINLINE void MemUsage(std::string name, float memory) noexcept
	{
		if(mem_usage.find(name) != mem_usage.end()) mem_usage[name] = memory;
		else mem_usage.emplace(name, memory);
	}

	OPENVIEWER_FORCEINLINE auto Start() const noexcept
	{
		return std::chrono::system_clock::now();
	}

	OPENVIEWER_FORCEINLINE auto End() const noexcept
	{
		return std::chrono::system_clock::now();
	}

	OPENVIEWER_FORCEINLINE void Time(const std::string name,
									 const std::chrono::time_point<std::chrono::system_clock>& start, 
			  						 const std::chrono::time_point<std::chrono::system_clock>& end) noexcept
	{
		const float time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
		
		if(times.find(name) != times.end()) times[name] = time;
		else times.emplace(name, time);
	}
};