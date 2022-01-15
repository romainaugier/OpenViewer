// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include <string>

#include "nlohmann/json.hpp"

#include "imgui.h"
#include "implaybar.h"
#include "ImFileDialog.h"
#include "utils/profiler.h"
#include "utils/filesystem_utils.h"
#include "utils/imgui_utils.h"
#include "core/ocio.h"
#include "display.h"

using Json = nlohmann::json;

// struct to hold the settings
struct Settings
{
	Json m_UserSettings;
	Json m_RuntimeSettings;

	std::vector<std::string> m_OcioConfigs;

	bool m_CacheSettingsChanged = false;

	void Initialize() noexcept;

	void LoadUserSettings() noexcept;

	void WriteUserSettings() noexcept;

	void InitializeRuntimeSettings() noexcept;

	void Close() noexcept;
};

// struct to hold the different windows for settings

namespace Interface
{
	struct Settings_Windows
	{
		Settings m_Settings;

		bool showSettingsWindow = false;
		bool showDebugWindow = false;

		void Draw(Profiler* prof, Logger* logger, Core::Ocio& ocio) noexcept;

		void GetOcioConfig(Core::Ocio& ocio) noexcept;
		
		// Few openers for windows
		void ShowSettingsWindow() noexcept { this->showSettingsWindow = !this->showSettingsWindow; }
		void ShowDebugWindow() noexcept { this->showDebugWindow = !this->showDebugWindow; }

		void Release() noexcept;
	};
} // End namespace Interface