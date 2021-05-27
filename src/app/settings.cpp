#include "settings.h"

void Settings::draw(ImPlaybar& playbar)
{	
	if (p_open_playback_window)
	{
		ImGui::SetNextWindowBgAlpha(interface_windows_bg_alpha);

		ImGui::Begin("Playback Settings");
		{
			ImGui::InputInt("FPS", &playbar.playbar_framerate);
		}
		ImGui::End();
	}

	if (p_open_ocio_window)
	{
		ImGui::SetNextWindowBgAlpha(interface_windows_bg_alpha);
	
		ImGui::Begin("OCIO Settings");
		{

		}
		ImGui::End();
	}

	if (p_open_interface_window)
	{
		ImGui::SetNextWindowBgAlpha(interface_windows_bg_alpha);
		
		ImGui::Begin("Interface Settings");
		{
			ImGui::InputFloat("Background Alpha", &interface_windows_bg_alpha);
		}
		ImGui::End();
	}

	if (p_open_performance_window)
	{
		ImGui::SetNextWindowBgAlpha(interface_windows_bg_alpha);
	
		ImGui::Begin("Performance Settings");
		{
			static int cache_size = 16;
			ImGui::InputInt("Cache Size (GB)", &cache_size);
		}
		ImGui::End();
	}
}