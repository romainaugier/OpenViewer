// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#include "menubar.h"

void Menubar::draw(Settings& current_settings, Loader& loader, Display& display, ImPlaybar& playbar, Ocio& ocio)
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
					loader.Release();
				}

				loader.Initialize(fp, 0, false);
				display.Initialize(loader, ocio);

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
					loader.Release();
				}

				const std::string fp = res.u8string();

				loader.Initialize(fp, 1000000, true);
				display.Initialize(loader, ocio);

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
			ImGui::Checkbox("Playback", &current_settings.p_open_playback_window);
			ImGui::Checkbox("OCIO", &current_settings.p_open_ocio_window);
			ImGui::Checkbox("Interface", &current_settings.p_open_interface_window);
			ImGui::Checkbox("Performance", &current_settings.p_open_performance_window);


			ImGui::EndMenu();
		}

		const ImVec2 avail_width = ImGui::GetContentRegionAvail();

		static int item_current = 0;

		ImGui::Dummy(ImVec2(avail_width.x - 200.0f, avail_width.y));

		ImGui::SetNextItemWidth(100.0f);
		ImGui::Combo("", &item_current, &ocio.active_views[0], ocio.active_views.size());

		ocio.current_view = ocio.active_views[item_current];
	}

	ImGui::EndMainMenuBar();
}