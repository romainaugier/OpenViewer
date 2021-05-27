#include "imgui.h"
#include "implaybar.h"

struct Settings
{
	float interface_windows_bg_alpha = 1.0f;

	// settings windows
	bool p_open_interface_window = false;
	bool p_open_ocio_window = false;
	bool p_open_playback_window = false;
	bool p_open_performance_window = false;


	void draw(ImPlaybar& playbar);
};