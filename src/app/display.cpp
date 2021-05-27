#include "display.h"

void Display::draw(GLuint tex_id, ImVec2 size)
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize;

	bool p_open = true;

	ImGui::Begin("Display", &p_open, window_flags);
	{
		static ImVec2 scrolling;
		static float zoom = 1.0f;
		const ImVec4 tint(1.0f, 1.0f, 1.0f, 1.0f);
		const ImVec4 border_color(0.0f, 0.0f, 0.0f, 1.0f);

		const bool is_hovered = ImGui::IsWindowHovered();

		ImGuiIO& io = ImGui::GetIO();

		if (is_hovered && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			scrolling.x += io.MouseDelta.x;
			scrolling.y += io.MouseDelta.y;
		}

		const float mouse_wheel = io.MouseWheel;

		if (is_hovered && mouse_wheel != 0)
		{
			zoom += mouse_wheel * 0.1f;
		}

		ImGui::SetCursorPos((ImGui::GetWindowSize() - size * zoom) * 0.5f + scrolling);
		ImGui::Image((void*)(intptr_t)tex_id, size * zoom, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), tint, border_color);
	}
	ImGui::End();
}