// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#include "menubar.h"

void Menubar::draw(Settings& current_settings, Loader& loader, Display& display, ImPlaybar& playbar, Ocio& ocio, Profiler& prof)
{
	ImGui::SetNextWindowBgAlpha(current_settings.interface_windows_bg_alpha);

	ImGui::BeginMainMenuBar();
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open Single File"))
			{
				ifd::FileDialog::Instance().Open("SingleFileOpenDialog", "Select an image file", "Image File (*.exr,*.png;*.jpg;*.jpeg;*.bmp;*.tga){.exr,.png,.jpg,.jpeg,.bmp,.tga},.*");
			}

			if (ImGui::MenuItem("Open Folder"))
			{
				ifd::FileDialog::Instance().Open("FolderOpenDialog", "Open a directory", "");
			}

			ImGui::EndMenu();
		}

		if (ifd::FileDialog::Instance().IsDone("SingleFileOpenDialog"))
		{
			if (ifd::FileDialog::Instance().HasResult())
			{
				const auto& res = ifd::FileDialog::Instance().GetResults();

				const std::string fp = res[0].u8string();

				if (loader.has_been_initialized > 0)
				{
					loader.stop_playloader = 1;

					loader.JoinWorker();
					loader.Release();
				}

				loader.Initialize(fp, 0, false, prof);
				display.Initialize(loader, ocio, prof);

				playbar.playbar_range = ImVec2(0.0f, loader.count + 1.0f);
				playbar.playbar_frame = 0;
			}

			ifd::FileDialog::Instance().Close();
		}

		if (ifd::FileDialog::Instance().IsDone("FolderOpenDialog"))
		{
			if (ifd::FileDialog::Instance().HasResult())
			{
				const auto& res = ifd::FileDialog::Instance().GetResult();

				if (loader.has_been_initialized > 0)
				{
					loader.stop_playloader = 1;

					loader.JoinWorker();
					loader.Release();
				}

				const std::string fp = res.u8string();

				loader.Initialize(fp, 2000000000, true, prof);
				loader.LaunchSequenceWorker();
				display.Initialize(loader, ocio, prof);

				playbar.playbar_range = ImVec2(0.0f, loader.count + 1.0f);

				playbar.playbar_frame = 0;
			}

			ifd::FileDialog::Instance().Close();
		}

		if (ImGui::BeginMenu("Plot"))
		{
			if (ImGui::MenuItem("Histogram")) {}
			if (ImGui::MenuItem("Waveform")) {}
			if (ImGui::MenuItem("Vector Scope")) {}
			if (ImGui::MenuItem("Custom")) {}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Settings"))
		{
			if (ImGui::MenuItem("Playback")) { current_settings.p_open_playback_window = true; }
			if (ImGui::MenuItem("OCIO")) { current_settings.p_open_ocio_window = true; }
			if (ImGui::MenuItem("Interface")) { current_settings.p_open_interface_window = true; }
			if (ImGui::MenuItem("Performance")) {current_settings.p_open_performance_window = true; }


			ImGui::EndMenu();
		}

		const ImVec2 avail_width = ImGui::GetContentRegionAvail();

		ImGui::Dummy(ImVec2(avail_width.x - 900.0f, avail_width.y));

		ImGui::Text("Role");
		ImGui::PushID(0);
		ImGui::SetNextItemWidth(200.0f);
		ImGui::Combo("", &ocio.current_role_idx, &ocio.roles[0], ocio.roles.size());
		ImGui::PopID();

		if (ImGui::IsItemEdited())
		{
			ocio.current_role = ocio.roles[ocio.current_role_idx];

			ocio.UpdateProcessor();
		}

		ImGui::Text("Display");

		ImGui::PushID(1);
		ImGui::SetNextItemWidth(200.0f);
		ImGui::Combo("", &ocio.current_display_idx, &ocio.active_displays[0], ocio.active_displays.size());
		ImGui::PopID();

		if (ImGui::IsItemEdited())
		{
			ocio.current_display = ocio.active_displays[ocio.current_display_idx];

			ocio.GetOcioDisplayViews();
			ocio.UpdateProcessor();
		}

		ImGui::Text("View");

		ImGui::PushID(2);
		ImGui::SetNextItemWidth(200.0f);
		ImGui::Combo("", &ocio.current_view_idx, &ocio.active_views[0], ocio.active_views.size());
		ImGui::PopID();

		if (ImGui::IsItemEdited())
		{
			ocio.current_view = ocio.active_views[ocio.current_view_idx]; 
			ocio.UpdateProcessor();
		}
	}

	ImGui::EndMainMenuBar();
}