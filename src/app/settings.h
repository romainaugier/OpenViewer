// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#pragma once

#include "imgui.h"
#include "implaybar.h"
#include "utils/profiler.h"

struct Settings
{
	float interface_windows_bg_alpha = 1.0f;

	// settings windows
	bool p_open_interface_window = false;
	bool p_open_ocio_window = false;
	bool p_open_playback_window = false;
	bool p_open_performance_window = false;


	void draw(ImPlaybar& playbar, Profiler& prof) noexcept;
};