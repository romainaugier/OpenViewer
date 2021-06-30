// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#include "settings.h"

void Settings::draw(ImPlaybar& playbar, Profiler& prof)
{	
	if (p_open_playback_window)
	{
		ImGui::SetNextWindowBgAlpha(interface_windows_bg_alpha);

		ImGui::Begin("Playback Settings", &p_open_playback_window);
		{
			ImGui::InputInt("FPS", &playbar.playbar_framerate);
		}
		ImGui::End();
	}

	if (p_open_ocio_window)
	{
		ImGui::SetNextWindowBgAlpha(interface_windows_bg_alpha);
	
		ImGui::Begin("OCIO Settings", &p_open_ocio_window);
		{

		}
		ImGui::End();
	}

	if (p_open_interface_window)
	{
		ImGui::SetNextWindowBgAlpha(interface_windows_bg_alpha);
		
		ImGui::Begin("Interface Settings", &p_open_interface_window);
		{
			ImGui::InputFloat("Background Alpha", &interface_windows_bg_alpha);
		}
		ImGui::End();
	}

	if (p_open_performance_window)
	{
		ImGui::SetNextWindowBgAlpha(interface_windows_bg_alpha);
	
		ImGui::Begin("Performance Settings", &p_open_performance_window);
		{
			ImGuiIO& io = ImGui::GetIO();

			static int cache_size = 16;
			ImGui::InputInt("Cache Size (GB)", &cache_size);
			ImGui::Text("Frame Average Time : %0.3f ms/frame (%0.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			ImGui::Text("Image Load Average Time : %0.3f ms", prof.avg_load_time);
			ImGui::Text("Ocio Transform Average Time : %0.3f ms", prof.avg_ocio_transform_time);
			ImGui::Text("Plot Average Time : %0.3f ms", prof.avg_plot_calc_time);
		}
		ImGui::End();
	}
}