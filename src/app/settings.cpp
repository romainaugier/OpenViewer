// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#include "settings.h"

// Little method to get the current ocio config path
void Settings_Windows::GetOcioConfig(Ocio& ocio) noexcept
{
	const std::string tmp = ocio.config_path;
	char* path = new char [tmp.size() + 1];
	memcpy(path, tmp.c_str(), tmp.size() + 1);
	settings.current_config = path;
	settings.configs.push_back(path);
}

void Settings_Windows::draw(ImPlaybar& playbar, Profiler* prof, Ocio& ocio, Loader& loader) noexcept
{	
	if (p_open_playback_window)
	{
		ImGui::SetNextWindowBgAlpha(settings.interface_windows_bg_alpha);

		ImGui::Begin("Playback Settings", &p_open_playback_window);
		{
			ImGui::InputInt("FPS", &playbar.playbar_framerate);
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
				ifd::FileDialog::Instance().Open("ConfigFileDialog", "Select an OCIO Config", "Ocio file (*.ocio){.oxio},.*");
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

			if (ImGui::CollapsingHeader("Cache"))
			{
				ImGui::Checkbox("Use Cache", &settings.use_cache);
				if (ImGui::IsItemEdited())
				{
					if (settings.use_cache)
					{
						if (loader.has_been_initialized > 0)
						{
							loader.mtx.lock();
							loader.stop_playloader = 1;
							loader.is_playloader_working = 0;
							loader.mtx.unlock();
							loader.load_into_cache.notify_all();
							
							std::this_thread::sleep_for(std::chrono::milliseconds(100));

							playbar.play = 0;
							playbar.playbar_frame = 0;
							loader.use_cache = 1;
							loader.cache_size = static_cast<uint64_t>(settings.cache_size) * 1000000;
							loader.ReleaseCache();
							loader.ReallocateCache(true);
							loader.LaunchSequenceWorker(true);
						}
						else
						{
							loader.use_cache = 1;
						}
					}
					else
					{
						if (loader.has_been_initialized > 0)
						{
							loader.mtx.lock();
							loader.stop_playloader = 1;
							loader.is_playloader_working = 0;
							loader.mtx.unlock();
							loader.load_into_cache.notify_all();

							std::this_thread::sleep_for(std::chrono::milliseconds(100));

							playbar.play = 0;
							playbar.playbar_frame = 0;
							loader.use_cache = 0;
							loader.cache_size = static_cast<uint64_t>(settings.cache_size) * 1000000;
							loader.ReleaseCache();
							loader.ReallocateCache(false);
							loader.LoadImage(0, loader.memory_arena);
						}
						else
						{
							loader.use_cache = 0;
						}
					}
				}
				
				ImGui::Text("Cache Size (MB)");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(100.0f);
				ImGui::InputInt("", &settings.cache_size, 0);
				ImGui::SameLine();
				if (ImGui::SmallButton("Set"))
				{
					if (settings.use_cache && loader.has_been_initialized > 0)
					{
						loader.mtx.lock();
						loader.stop_playloader = 1;
						loader.is_playloader_working = 0;
						loader.mtx.unlock();
						loader.load_into_cache.notify_all();
						
						std::this_thread::sleep_for(std::chrono::milliseconds(100));
						
						playbar.play = 0;
						playbar.playbar_frame = 0;
						loader.cache_size = static_cast<uint64_t>(settings.cache_size) * 1000000;
						loader.ReleaseCache();
						loader.ReallocateCache(true);
						loader.LaunchSequenceWorker(true);
					}
					else
					{
						loader.cache_size = static_cast<uint64_t>(settings.cache_size) * 1000000;
					}
				}
			}

			if (ImGui::CollapsingHeader("Profiler"))
			{
				ImGui::Text("Speed");
				ImGui::Text("Frame Average Time : %0.3f ms/frame (%0.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
				ImGui::Text("Image Load Average Time : %0.3f ms", prof->avg_load_time);
				ImGui::Text("Ocio Transform Average Time : %0.3f ms", prof->avg_ocio_transform_time);
				ImGui::Text("Unpacking Average Time : %0.3f ms", prof->avg_unpack_calc_time);
				ImGui::Text("Memory");
				ImGui::Text("Current Memory Usage : %0.3f MB", prof->current_memory_usage);
				ImGui::Text("Loader : %0.6f MB", prof->loader_size);
				ImGui::Text("Display : %0.6f MB", prof->display_size);
				ImGui::Text("Ocio : %0.6f MB", prof->ocio_size);
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