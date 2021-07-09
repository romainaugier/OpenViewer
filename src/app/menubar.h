// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#pragma once

#include "settings.h"
#include "core/loader.h"
#include "ImFileDialog.h"
#include "display.h"
#include "implaybar.h"
#include "core/ocio.h"

#include "stdio.h"
#include <algorithm>

struct Menubar
{
	int bar_mode = 0;
	unsigned int has_opened_ifd : 1;

	Menubar()
	{
		has_opened_ifd = 0;
	}

	void draw(Settings_Windows& current_settings, Loader& loader, Display& display, 
			  ImPlaybar& playbar, Ocio& ocio, Profiler& prof, bool& change) noexcept;
};