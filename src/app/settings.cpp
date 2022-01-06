// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#include "settings.h"

namespace Interface
{
	// Little method to get the current ocio config path
	void Settings_Windows::GetOcioConfig(Core::Ocio& ocio) noexcept
	{
		const std::string tmp = ocio.config_path;
		char* path = new char [tmp.size() + 1];
		memcpy(path, tmp.c_str(), tmp.size() + 1);
		settings.current_config = path;
		settings.configs.push_back(path);
	}

	void Settings_Windows::Draw(ImPlaybar& playbar, Profiler* prof, Core::Ocio& ocio, Application& app) noexcept
	{	
		if (showPlaybackWindow)
		{
			ImGui::Begin("Playback Settings", &showPlaybackWindow);
			{
				ImGui::InputInt("FPS", (int*)&playbar.m_FrameRate);
			}
			ImGui::End();
		}

		if (showOcioWindow)
		{
			ImGui::Begin("OCIO Settings", &showOcioWindow);
			{
				ImGui::Text("Current configuration : %s", settings.current_config);
				ImGui::Text("");
				ImGui::Text("Configurations : ");
				ImGui::SameLine();
				ImGui::PushID(0);
				ImGui::Combo("", &settings.current_config_idx, &settings.configs[0], settings.configs.size());
				ImGui::PopID();
				{
					if (ImGui::IsItemEdited())
					{
						settings.current_config = settings.configs[settings.current_config_idx];
						ocio.ChangeConfig(settings.configs[settings.current_config_idx]);
					}
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Add"))
				{
					ifd::FileDialog::Instance().Open("ConfigFileDialog", "Select an OCIO Config", "Ocio file (*.ocio){.ocio},.*");
				}
			}

			if (ifd::FileDialog::Instance().IsDone("ConfigFileDialog"))
			{
				if (ifd::FileDialog::Instance().HasResult())
				{
					const auto& res = ifd::FileDialog::Instance().GetResults();
					const std::string fp = res[0].u8string();

					char* tmp = new char[fp.size() + 1];
					memcpy(tmp, fp.c_str(), fp.size() + 1);

					settings.configs.push_back(tmp);
				}

				ifd::FileDialog::Instance().Close();
			}

			ImGui::End();
		}

		if (showInterfaceWindow)
		{
			ImGui::Begin("Interface Settings", &showInterfaceWindow);
			{
				ImGui::InputFloat("Background Alpha", &settings.interface_windows_bg_alpha);
				if (ImGui::IsItemEdited())
				{
					ImVec4* colors = ImGui::GetStyle().Colors;
    				ImVec4 tmpColor = colors[ImGuiCol_WindowBg];
					tmpColor.w = settings.interface_windows_bg_alpha;
					colors[ImGuiCol_WindowBg] = tmpColor;
				}
			}
			ImGui::End();
		}

		if (showDebugWindow)
		{
			ImGui::Begin("Debug", &showDebugWindow);
			{
				// Log Level
				const char* logLevels[6] = { "Error", "Warning", "Message", "Verbose", "Diagnostic", "Debug" };
				
				static int logLevelIndex = 1;

				ImGui::Text("Log Level");

				ImGui::SameLine();

				const ImVec2 leftSpace = ImGui::GetContentRegionAvail();

				ImGui::Dummy(ImVec2(leftSpace.x - 100.0f, 1.0f));
				ImGui::SameLine();

				ImGui::SetNextItemWidth(100.0f);
				ImGui::Combo("###", &logLevelIndex, logLevels, 6);

				if (ImGui::IsItemEdited())
				{
					if (logLevelIndex == 0) app.m_Logger->SetLevel(LogLevel_Error);
					if (logLevelIndex == 1) app.m_Logger->SetLevel(LogLevel_Warning);
					if (logLevelIndex == 2) app.m_Logger->SetLevel(LogLevel_Message);
					if (logLevelIndex == 3) app.m_Logger->SetLevel(LogLevel_Verbose);
					if (logLevelIndex == 4) app.m_Logger->SetLevel(LogLevel_Diagnostic);
					if (logLevelIndex == 5) app.m_Logger->SetLevel(LogLevel_Debug);
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
		for (auto& conf : settings.configs)
		{
			delete[] conf;
			conf = nullptr;
		}
	}
} // End namespace Interface