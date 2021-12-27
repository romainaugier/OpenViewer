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
		if (p_open_playback_window)
		{
			ImGui::SetNextWindowBgAlpha(settings.interface_windows_bg_alpha);

			ImGui::Begin("Playback Settings", &p_open_playback_window);
			{
				ImGui::InputInt("FPS", (int*)&playbar.m_FrameRate);
			}
			ImGui::End();
		}

		if (p_open_ocio_window)
		{
			ImGui::SetNextWindowBgAlpha(settings.interface_windows_bg_alpha);
		
			ImGui::Begin("OCIO Settings", &p_open_ocio_window);
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

		if (p_open_interface_window)
		{
			ImGui::SetNextWindowBgAlpha(settings.interface_windows_bg_alpha);
			
			ImGui::Begin("Interface Settings", &p_open_interface_window);
			{
				ImGui::InputFloat("Background Alpha", &settings.interface_windows_bg_alpha);
			}
			ImGui::End();
		}

		if (p_open_performance_window)
		{
			ImGui::SetNextWindowBgAlpha(settings.interface_windows_bg_alpha);
		
			ImGui::Begin("Performance Settings", &p_open_performance_window);
			{
				ImGuiIO& io = ImGui::GetIO();

				if (ImGui::CollapsingHeader("Profiling"))
				{
					ImGui::Text("Time");
					ImGui::Text("Frame Average Time : %0.3f ms/frame (%0.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
					for(const auto& [key, value] : prof->times)	ImGui::Text("%s : %0.3f ms", key.c_str(), value);
					ImGui::Separator();
					ImGui::Text("Memory");
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