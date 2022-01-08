// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include "settings.h"
#include "core/loader.h"
#include "ImFileDialog.h"
#include "display.h"
#include "implaybar.h"
#include "imginfos.h"
#include "core/ocio.h"

#include "stdio.h"
#include <algorithm>

namespace Interface
{
	struct Settings_Windows;
	struct Application;

	struct Menubar
	{
		int m_BarMode = 0;

		uint8_t m_ModeCount = 4;

		bool m_HasOpenedIFD = false;

		void Draw(Settings_Windows& currentSettings, 
				  Application& app,
				  ImPlaybar& playbar, 
				  Core::Ocio& ocio, 
				  Profiler& prof, 
				  bool& change) noexcept;

		void SetMode(const uint8_t mode) noexcept { this->m_BarMode = mode; }
		uint8_t GetMode() const noexcept { return static_cast<uint8_t>(this->m_BarMode); }
		uint8_t GetModeCount() const noexcept { return this->m_ModeCount; }
	};
} // End namespace Interface