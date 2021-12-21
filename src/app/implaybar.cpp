// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "implaybar.h"

namespace Interface
{
	void ImPlaybar::Draw() noexcept
	{
		bool p_open = true;

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | 
										ImGuiWindowFlags_NoMove | 
										ImGuiWindowFlags_NoNav | 
										ImGuiWindowFlags_NoScrollbar | 
										ImGuiWindowFlags_NoCollapse |
										ImGuiWindowFlags_NoResize;

		constexpr float playbarHeight = 80.0f;

		ImGui::SetNextWindowBgAlpha(0.3f);
		ImGui::SetNextWindowPos(ImVec2(0.0f, ImGui::GetIO().DisplaySize.y - playbarHeight));
		ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, playbarHeight));

		ImGui::Begin("Playbar", &p_open, window_flags);
		{	
			ImDrawList* drawList = ImGui::GetWindowDrawList();

			ImVec2 canvasP0 = ImGui::GetCursorScreenPos();
			ImVec2 canvasSize = ImGui::GetContentRegionAvail() - ImVec2(502.0f, 0.0f);
			ImVec2 canvasP1 = ImVec2(canvasP0.x + canvasSize.x, canvasP0.y + canvasSize.y);

			ImGui::InvisibleButton("Playbar", canvasSize);

			drawList->AddRectFilled(canvasP0, canvasP1, DARKGRAY);

			drawList->PushClipRect(canvasP0, canvasP1, true);

			const float step = canvasSize.x / (this->m_Range.y - 1.0f);

			// Separating lines and frame number indicators
			for (float x = 0.0f; x < (this->m_Range.y); x++)
			{
				float thickness = 1.0f;
				float height = (canvasP0.y + 10.0f);
				
				if (fmodf(x + 1.0f, 24.0f) == 0.0f || x == 0.0f)
				{
					thickness = 4.0f;
					height = canvasP1.y;

					char frameIndexText[16];
					sprintf(frameIndexText, "%d", (int)x + 1);

					
					drawList->AddText(nullptr, 15.0f, ImVec2(canvasP0.x + (step * x) + 8.0f, canvasP0.y + 12.5f), LIGHTGRAY, frameIndexText);
				}

				drawList->AddLine(ImVec2(canvasP0.x + (step * x) + 2.0f, canvasP0.y), ImVec2(canvasP0.x + (step * x) + 2.0f, height), LIGHTGRAY, thickness);
			}	

			// Cursor
			drawList->AddLine(ImVec2(canvasP0.x + (step * (this->m_Frame)), canvasP0.y), ImVec2(canvasP0.x + (step * (this->m_Frame)), canvasP1.y), LIGHTBLUE, 4.0f);

			drawList->PopClipRect();	
		}
		ImGui::End();
	}
} // End namespace Interface