// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#include "implaybar.h"

void ImPlaybar::draw(std::vector<char>& cached, bool& change) noexcept
{
	bool p_open = true;

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | 
									ImGuiWindowFlags_NoMove | 
									ImGuiWindowFlags_NoNav | 
									ImGuiWindowFlags_NoScrollbar | 
									ImGuiWindowFlags_NoCollapse |
									ImGuiWindowFlags_NoResize;

	if (play > 0)
	{
		uint32_t framecount = ImGui::GetFrameCount();
		uint32_t fps = ImGui::GetIO().Framerate;
		float time = (float)fps / (float)playbar_framerate;

		if (fmodf((float)framecount, time) < 1.0f)
		{
			playbar_frame++;
			playbar_frame = fmodf(playbar_frame, (playbar_range.y - 1.0f));
			update = 1;
		}
		else
		{
			update = 0;
		}
	}

	ImGui::SetNextWindowBgAlpha(0.3f);
	ImGui::SetNextWindowPos(ImVec2(0.0f, ImGui::GetIO().DisplaySize.y - 30.0f));
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 30.0f));

	ImGui::Begin("Playbar", &p_open, window_flags);
	{
		static float scrolling = 0;

		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
		ImVec2 canvas_size = ImGui::GetContentRegionAvail() - ImVec2(500.0f, 0.0f);
		ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_size.x, canvas_p0.y + canvas_size.y);

		ImGui::InvisibleButton("Playbar", canvas_size);

		draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(10, 10, 10, 40));

		draw_list->PushClipRect(canvas_p0, canvas_p1, true);

		const float step = canvas_size.x / (playbar_range.y - 1.0f);

		// separating lines
		for (float x = 0.0f; x < (playbar_range.y); x++)
		{
			float thickness = 1.0f;
			float height = (canvas_p0.y + canvas_p1.y) / 2.0f;
			
			if (fmodf(x + 1.0f, 24.0f) == 0.0f || x == 0.0f)
			{
				thickness = 4.0f;
				height = canvas_p1.y;
			}

			draw_list->AddLine(ImVec2(canvas_p0.x + (step * x), canvas_p0.y), ImVec2(canvas_p0.x + (step * x), height), IM_COL32(200, 200, 200, 40), thickness);
		}

		// cursor
		draw_list->AddRectFilled(ImVec2(canvas_p0.x + (step * (playbar_frame)), canvas_p0.y), ImVec2(canvas_p0.x + (step * (playbar_frame + 1)), canvas_p1.y), IM_COL32(200, 200, 200, 128));

		draw_list->PopClipRect();

		// cache indices
		for (float x = 0.0f; x < cached.size(); x++)
		{
			if (cached[x] > 0)
			{
				float height = canvas_p0.y + (canvas_p0.y + canvas_p1.y) / 2.0f;
				draw_list->AddRectFilled(ImVec2(canvas_p0.x + (step * x), height), ImVec2(canvas_p0.x + (step * (x + 1)), canvas_p1.y), IM_COL32(0, 255, 0, 155));
			}
		}

		// click to set frame freely
		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			float x_position = ImGui::GetIO().MouseClickedPos->x - ImGui::GetCursorScreenPos().x - ImGui::GetScrollX();
			playbar_frame = floor(x_position / step);
			scrolling = x_position;
			play = 0;
		}

		// dragging and play frames dragged
		if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			scrolling += ImGui::GetIO().MouseDelta.x;
			scrolling = scrolling < 0 ? 0.0f : scrolling;
			scrolling = scrolling > (playbar_range.y - 1.0f) * step ? (playbar_range.y - 2.0f) * step : scrolling;
			playbar_frame = scrolling / step;
			play = 0;
		}

		// buttons
		const ImVec2 buttons_size = ImVec2(50.0f, 15.0f);

		ImGui::SameLine();
		if (ImGui::Button(u8"⮜⮜", buttons_size)) playbar_frame = playbar_range.x;
		ImGui::SameLine();
		if (ImGui::Button(u8"⮞", buttons_size)) play = 1;
		ImGui::SameLine();
		if (ImGui::Button(u8"■", buttons_size)) play = 0;
		ImGui::SameLine();
		if (ImGui::Button(u8"⮞⮞", buttons_size)) playbar_frame = playbar_range.y - 2;

		/*

		const float tri_size = 30.0f;
		const float padding = 10.0f;

		// dbl arrows left button
		bool is_dbl_arr_lft_hovered = hover(ImVec2(canvas_p1.x + padding, canvas_p0.y), 
											ImVec2(canvas_p1.x + (padding + tri_size) + 15.0f, canvas_p1.y),
											ImGui::GetMousePos());

		ImU32 color1 = IM_COL32(100, 100, 100, 255);

		if (is_dbl_arr_lft_hovered)
		{
			color1 = IM_COL32(200, 200, 200, 255);
			if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)) playbar_frame = playbar_range.x;
		}

		draw_list->AddTriangleFilled(ImVec2(canvas_p1.x + padding, (canvas_p0.y + canvas_p1.y) / 2.0f), 
									 ImVec2(canvas_p1.x + (padding + tri_size), canvas_p0.y), 
									 ImVec2(canvas_p1.x + (padding + tri_size), canvas_p1.y), color1);
		draw_list->AddTriangleFilled(ImVec2(canvas_p1.x + padding + 15.0f, (canvas_p0.y + canvas_p1.y) / 2.0f),
									 ImVec2(canvas_p1.x + (padding + tri_size) + 15.0f, canvas_p0.y),
									 ImVec2(canvas_p1.x + (padding + tri_size) + 15.0f, canvas_p1.y), color1);

		// play button
		float offset = (padding) + 15.0f;

		bool is_play_hovered = hover(ImVec2(canvas_p1.x + offset + 45.0f, canvas_p0.y),
			ImVec2(canvas_p1.x + (offset + tri_size) + 45.0f, canvas_p1.y),
			ImGui::GetMousePos());

		ImU32 color2 = IM_COL32(100, 100, 100, 255);

		if (is_play_hovered)
		{
			color2 = IM_COL32(200, 200, 200, 255);
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) play = 1;
		}

		draw_list->AddTriangleFilled(ImVec2(canvas_p1.x + 80.0f + offset, (canvas_p0.y + canvas_p1.y) / 2.0f),
									 ImVec2(canvas_p1.x + 50.0f + offset, canvas_p0.y),
									 ImVec2(canvas_p1.x + 50.0f + offset, canvas_p1.y), color2);

		// stop button
		offset = (padding + tri_size * 1.5f) + 15.0f;

		bool is_stop_hovered = hover(ImVec2(canvas_p1.x + offset + 45.0f, canvas_p0.y),
			ImVec2(canvas_p1.x + (offset + tri_size) + 15.0f + 45.0f, canvas_p1.y),
			ImGui::GetMousePos());

		ImU32 color3 = IM_COL32(100, 100, 100, 255);

		if (is_stop_hovered)
		{
			color3 = IM_COL32(200, 200, 200, 255);
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) play = false;
		}

		draw_list->AddRectFilled(ImVec2(canvas_p1.x + 80.0f + offset, canvas_p0.y),
								 ImVec2(canvas_p1.x + 50.0f + offset, canvas_p1.y), color3);
		
		// dbl arrows right button
		offset = (padding + tri_size * 3.0f) + 15.0f;

		bool is_dbl_arr_rgt_hovered = hover(ImVec2(canvas_p1.x + offset + 45.0f, canvas_p0.y),
			ImVec2(canvas_p1.x + (offset + tri_size) + 15.0f + 45.0f, canvas_p1.y),
			ImGui::GetMousePos());

		ImU32 color4 = IM_COL32(100, 100, 100, 255);

		if (is_dbl_arr_rgt_hovered)
		{
			color4 = IM_COL32(200, 200, 200, 255);
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) playbar_frame = playbar_range.y - 2;
		}

		draw_list->AddTriangleFilled(ImVec2(canvas_p1.x + 80.0f + offset, (canvas_p0.y + canvas_p1.y) / 2.0f),
									 ImVec2(canvas_p1.x + 50.0f + offset, canvas_p0.y),
									 ImVec2(canvas_p1.x + 50.0f + offset, canvas_p1.y), color4);
		draw_list->AddTriangleFilled(ImVec2(canvas_p1.x + 80.0f + 15.0f + offset, (canvas_p0.y + canvas_p1.y) / 2.0f),
									 ImVec2(canvas_p1.x + 50.0f + 15.0f + offset, canvas_p0.y),
									 ImVec2(canvas_p1.x + 50.0f + 15.0f + offset, canvas_p1.y), color4);

		*/
		

	}

	ImGui::End();
}