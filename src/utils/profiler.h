// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include <chrono>
#include <string>
#include "tsl/robin_map.h"

#include "decl.h"

// Very simple profiler to get timings of some actions at runtime to help finding hotspots in the application

// There is also a simple map to get the memory usage of different structures used by OpenViewer, to help
// see what is consuming more or less memory

struct Profiler
{
	tsl::robin_map<std::string, float> times;
	tsl::robin_map<std::string, float> mem_usage;

	OV_FORCEINLINE void MemUsage(const std::string& name, float memory) noexcept
	{
		if(mem_usage.find(name) != mem_usage.end()) mem_usage[name] = memory;
		else mem_usage.emplace(name, memory);
	}

	OV_FORCEINLINE auto Start() const noexcept
	{
		return std::chrono::system_clock::now();
	}
	
	OV_FORCEINLINE OV_STATIC_FUNC auto StaticStart() noexcept
	{
		return std::chrono::system_clock::now();
	}

	OV_FORCEINLINE auto End() const noexcept
	{
		return std::chrono::system_clock::now();
	}
	
	OV_FORCEINLINE OV_STATIC_FUNC auto StaticEnd() noexcept
	{
		return std::chrono::system_clock::now();
	}

	OV_FORCEINLINE void Time(const std::string& name,
									 const std::chrono::time_point<std::chrono::system_clock>& start, 
			  						 const std::chrono::time_point<std::chrono::system_clock>& end) noexcept
	{
		const float time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
		
		times[name] = time;
	}
	
	OV_FORCEINLINE OV_STATIC_FUNC float StaticTime(const std::chrono::time_point<std::chrono::system_clock>& start, 
			  						 						 	   const std::chrono::time_point<std::chrono::system_clock>& end) noexcept
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	}
};