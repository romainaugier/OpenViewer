// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "settings.h"

void Settings::LoadUserSettings() noexcept
{
	const std::string ovDirPath = Utils::Fs::GetDocumentFolder() + "/OpenViewer";

	if (!std::filesystem::exists(ovDirPath)) std::filesystem::create_directory(ovDirPath);

	const std::string ovSettingsPath = ovDirPath + "/settings.json";

	if (!std::filesystem::exists(ovSettingsPath))
	{
		this->m_UserSettings = 
		{
			// Cache
			{"cache_mode", 0}, // 0 : Minimal, 1 : Manual, 2 : Smart
			{"cache_max_size", 0}, // Size is in MB
			{"cache_max_ram_usage", 50}, // Max percentage of ram to use in smart mode
			// I/O
			{"autodetect_file_sequences", true},
			// Performance
			{"openexr_threads", 8}
		};

		return;
	}

	std::ifstream i(ovSettingsPath);
	i >> this->m_UserSettings;
}

void Settings::WriteUserSettings() noexcept
{
	const std::string ovDirPath = Utils::Fs::GetDocumentFolder() + "/OpenViewer";

	if (!std::filesystem::exists(ovDirPath)) std::filesystem::create_directory(ovDirPath);

	const std::string ovSettingsPath = ovDirPath + "/settings.json";

	std::ofstream o(ovSettingsPath);
	o << std::setw(4) << this->m_UserSettings << std::endl;
}

void Settings::InitializeRuntimeSettings() noexcept
{
	this->m_RuntimeSettings = 
	{
		// Playback settings
		{"playback_speed", 24},
		// Logging
		{"log_level", 1}, // Warning by default
		// OCIO
		{"current_ocio_config_idx", 0}
	};
}

void Settings::Initialize() noexcept
{
	this->InitializeRuntimeSettings();
	this->LoadUserSettings();
}

void Settings::Close() noexcept
{
	this->WriteUserSettings();
}

namespace Interface
{
	// Little method to get the current ocio config path
	void Settings_Windows::GetOcioConfig(Core::Ocio& ocio) noexcept
	{
		this->m_Settings.m_OcioConfigs.push_back(ocio.m_ConfigPath);
	}

	void Settings_Windows::Draw(Profiler* prof, Logger* logger, Core::Ocio& ocio) noexcept
	{	
		if (showSettingsWindow)
		{
			ImGui::Begin("Settings", &showSettingsWindow);
			{
				// Cache
				if (ImGui::CollapsingHeader("Cache"))
				{
					// Cache mode
					const char* cacheModes[3] = { "Minimal", "Manual", "Smart" };
					int cacheMode = this->m_Settings.m_UserSettings["cache_mode"].get<int>();

					ImGui::Text("Cache Mode");

					ImGui::SameLineWidget(100.0f);

					IM_ID(0, ImGui::Combo("###", &cacheMode, cacheModes, 3));

					if (ImGui::IsItemEdited())
					{
						this->m_Settings.m_UserSettings["cache_mode"] = cacheMode;
						this->m_Settings.m_CacheSettingsChanged = true;
					}

					// Cache size
					if (cacheMode == 1)
					{
						int cacheSizeMB = this->m_Settings.m_UserSettings["cache_max_size"].get<int>();
						
						ImGui::Text("Cache Size");

						ImGui::SameLineWidget(100.0f);
						
						IM_ID(1, ImGui::InputInt("###", &cacheSizeMB, 0));

						if (ImGui::IsItemDeactivatedAfterEdit())
						{
							this->m_Settings.m_UserSettings["cache_max_size"] = cacheSizeMB;
							this->m_Settings.m_CacheSettingsChanged = true;
						}
					}
					else if (cacheMode == 2)
					{
						int percentageOfRamToUse = this->m_Settings.m_UserSettings["cache_max_ram_usage"].get<int>();

						ImGui::Text("%% Of Total Ram To Use");

						ImGui::SameLineWidget(100.0f);

						IM_ID(1, ImGui::InputInt("###", &percentageOfRamToUse, 0));

						if (ImGui::IsItemDeactivatedAfterEdit())
						{
							this->m_Settings.m_UserSettings["cache_max_ram_usage"] = percentageOfRamToUse;
							this->m_Settings.m_CacheSettingsChanged = true;
						}
					}
				}

				// I/O
				if (ImGui::CollapsingHeader("I/O"))
				{
					// Autodetect file sequence
					bool autodetect = this->m_Settings.m_UserSettings["autodetect_file_sequences"].get<bool>();

					ImGui::Text("Autodetect Sequences");

					ImGui::SameLineWidget(20.0f);

					IM_ID(2, ImGui::Checkbox("###", &autodetect));

					if (ImGui::IsItemEdited())
					{
						this->m_Settings.m_UserSettings["autodetect_file_sequences"] = autodetect;
					}
				}

				// Performance
				if (ImGui::CollapsingHeader("Performance"))
				{
					// OpenExr threads
					int numThreads = this->m_Settings.m_UserSettings["openexr_threads"].get<int>();

					ImGui::Text("OpenEXR Thread Count");

					ImGui::SameLineWidget(50.0f);

					IM_ID(3, ImGui::InputInt("###", &numThreads, 0));

					if (ImGui::IsItemEdited())
					{
						this->m_Settings.m_UserSettings["openexr_threads"] = numThreads;
					}
				}

				// OCIO
				if (ImGui::CollapsingHeader("OCIO"))
				{
					int currentOcioConfigIdx = this->m_Settings.m_RuntimeSettings["current_ocio_config_idx"].get<int>();

					ImGui::Text("Current Configuration : %s", this->m_Settings.m_OcioConfigs[currentOcioConfigIdx].c_str());

					IM_ID(4, ImGui::Combo("###", &currentOcioConfigIdx, 
								 [](void* vec, int idx, const char** out_text){
								 	std::vector<std::string>* vector = reinterpret_cast<std::vector<std::string>*>(vec);
								 	if (idx < 0 || idx >= vector ->size()) return false;
								 	*out_text = vector->at(idx).c_str();
								 	return true;
								 }, reinterpret_cast<void*>(&this->m_Settings.m_OcioConfigs), this->m_Settings.m_OcioConfigs.size()));

					if (ImGui::IsItemEdited())
					{
						this->m_Settings.m_RuntimeSettings["current_ocio_config_idx"] = currentOcioConfigIdx;

						ocio.ChangeConfig(this->m_Settings.m_OcioConfigs[currentOcioConfigIdx].c_str());
					}

					ImGui::SameLine();

					if (ImGui::SmallButton("Add"))
					{
						ifd::FileDialog::Instance().Open("ConfigFileDialog", "Select an OCIO Config", "Ocio file (*.ocio){.ocio},.*");
					}
				}

				// Interface
				if (ImGui::CollapsingHeader("Interface"))
				{
					ImGui::Text("Windows Opacity");

					ImGui::SameLineWidget(120.0f);

					IM_ID(5, ImGui::SliderFloat("###", &ImGui::GetStyle().Colors[ImGuiCol_WindowBg].w, 0.0f, 1.0f));
				}
			}

			if (ifd::FileDialog::Instance().IsDone("ConfigFileDialog"))
			{
				if (ifd::FileDialog::Instance().HasResult())
				{
					const auto& res = ifd::FileDialog::Instance().GetResults();
					const std::string fp = res[0].u8string();

					this->m_Settings.m_OcioConfigs.push_back(fp);
				}

				ifd::FileDialog::Instance().Close();
			}

			ImGui::End();
		}

		if (showDebugWindow)
		{
			ImGui::Begin("Debug", &showDebugWindow);
			{
				// Log Level
				const char* logLevels[6] = { "Error", "Warning", "Message", "Verbose", "Diagnostic", "Debug" };
				
				int logLevelIndex = this->m_Settings.m_RuntimeSettings["log_level"].get<int>();

				ImGui::Text("Log Level");

				ImGui::SameLineWidget(100.0f);

				IM_ID(5, ImGui::Combo("###", &logLevelIndex, logLevels, 6));

				if (ImGui::IsItemEdited())
				{
					this->m_Settings.m_RuntimeSettings["log_level"] = logLevelIndex;

					if (logLevelIndex == 0) logger->SetLevel(LogLevel_Error); 
					if (logLevelIndex == 1) logger->SetLevel(LogLevel_Warning);
					if (logLevelIndex == 2) logger->SetLevel(LogLevel_Message);
					if (logLevelIndex == 3) logger->SetLevel(LogLevel_Verbose);
					if (logLevelIndex == 4) logger->SetLevel(LogLevel_Diagnostic);
					if (logLevelIndex == 5) logger->SetLevel(LogLevel_Debug);
				}

				ImGuiIO& io = ImGui::GetIO();

				if (ImGui::CollapsingHeader("Event Profiling"))
				{
					ImGui::Text("Frame Average Time : %0.3f ms/frame (%0.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
					ImGui::Separator();
					for(const auto& [key, value] : prof->times)	ImGui::Text("%s : %0.3f ms", key.c_str(), value / 1000.0f);
				}
				if (ImGui::CollapsingHeader("Memory Profiling"))
				{
					for(const auto& [key, value] : prof->mem_usage) ImGui::Text("%s : %0.3f MB", key.c_str(), value);
				}
			}
			ImGui::End();
		}
	}

	void Settings_Windows::Release() noexcept
	{
		this->m_Settings.m_OcioConfigs.clear();
	}
} // End namespace Interface