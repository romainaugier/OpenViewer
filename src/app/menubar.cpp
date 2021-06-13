#include "menubar.h"

void Menubar::draw(Settings& current_settings, Loader& loader, Display& display, ImPlaybar& playbar)
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

				std::string fp = res[0].u8string();

				if (loader.has_been_initialized > 0)
				{
					loader.release();
				}

				loader.initialize(fp, 0, false);
				display.init(loader);

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
					loader.release();
				}

				std::string fp = res.u8string();

				loader.initialize(fp, 1000000, true);
				display.init(loader);

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
	}

	ImGui::EndMainMenuBar();
}