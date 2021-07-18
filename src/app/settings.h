// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#pragma once

#include <string>

#include "imgui.h"
#include "implaybar.h"
#include "ImFileDialog.h"
#include "utils/profiler.h"
#include "core/ocio.h"
#include "display.h"

// struct to hold the settings
struct Settings
{
	// cache settings
	bool use_cache = false;
	int cache_size = 500;

	// interface settings
	float interface_windows_bg_alpha = 1.0f;

	// ocio
	const char* current_config = "";
	int current_config_idx = 0;
	std::vector<const char*> configs;

	// plot
	bool parade = false;
};

// struct to hold the different windows for settings
struct Settings_Windows
{
	Settings settings;

	bool p_open_interface_window = false;
	bool p_open_ocio_window = false;
	bool p_open_playback_window = false;
	bool p_open_performance_window = false;

	void draw(ImPlaybar& playbar, Profiler* prof, Ocio& ocio, Loader& loader) noexcept;
	void GetOcioConfig(Ocio& ocio) noexcept;
	void Release() noexcept;
};