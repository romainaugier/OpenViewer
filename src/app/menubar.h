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

namespace Interface
{
	struct Menubar
	{
		int m_BarMode = 0;
		bool m_HasOpenedIFD = false;

		void Draw(Settings_Windows& currentSettings, 
				  Application& app,
				  ImPlaybar& playbar, 
				  Core::Ocio& ocio, 
				  Profiler& prof, 
				  bool& change) noexcept;
	};
} // End namespace Interface