#include "menubar.h"

void Menubar::draw(Settings& current_settings)
{
	ImGui::SetNextWindowBgAlpha(current_settings.interface_windows_bg_alpha);

	ImGui::BeginMainMenuBar();
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open Single File"))
			{
				printf("Open file\n");
			}

			if (ImGui::MenuItem("Open Image Sequence"))
			{
				printf("Open image sequence\n");
			}

			if (ImGui::MenuItem("Open Folder"))
			{
				printf("Open folder\n");
			}

			ImGui::EndMenu();
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
	}

	ImGui::EndMainMenuBar();
}