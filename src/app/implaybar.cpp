// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "implaybar.h"

namespace Interface
{
	void ImPlaybar::Play() noexcept { this->m_Play = true; }

	void ImPlaybar::Pause() noexcept { this->m_Play = false; }

	void ImPlaybar::Update(Profiler* profiler) noexcept
	{
		if (this->m_Loader->m_HasBeenInitialized)
		{
			// Update the range of the loader to match the one of the playbar
			this->m_Loader->m_Range = this->m_Range;

			if (this->m_Play)
			{
				// Update frame if we are playing
				const uint32_t framecount = ImGui::GetFrameCount();
				const uint32_t fps = ImGui::GetIO().Framerate;
				const float time = static_cast<float>(fps) / static_cast<float>(this->m_FrameRate);

				if (fmodf(static_cast<float>(framecount), time) < 1.0f)
				{
					this->m_Frame = fmodf(this->m_Frame + 1, (this->m_Range.y - 1.0f));
					this->m_Update = true;
				}
				else
				{
					this->m_Update = false;
				}

				if (this->m_Update)
				{
					const auto playbarUpdateStart = profiler->Start();
					
					// Notify the background cache loader that we need to store some frames in the cache
					if (this->m_Loader->m_UseCache)
					{
						// Urgent load in case of bug
						// this->m_Loader->LoadImageToCache(this->m_Frame);

						const uint32_t offset = this->m_Loader->m_Cache->m_Size - this->m_Loader->m_BgLoadChunkSize * 2;
						const uint32_t frameIdx = static_cast<uint32_t>(fmodf((this->m_Frame + offset), (this->m_Range.y - 1)));

						if (!this->m_CachedIndices[frameIdx])
						{
							std::unique_lock<std::mutex> playbarLock(this->m_Loader->m_Mutex);

							this->m_Loader->m_NeedBgLoad = true;
							this->m_Loader->m_BgLoadFrameIndex = frameIdx;

							playbarLock.unlock();

							this->m_Loader->m_CondVar.notify_all();
						}
					}
					else
					{
						this->m_Loader->LoadImageToCache(this->m_Frame);
					}
					
					const auto playbarUpdateEnd = profiler->End();
					
					profiler->Time("Playbar Update Time", playbarUpdateStart, playbarUpdateEnd);
				}
			}
			else
			{
				if (this->m_Update)
				{
					const auto playbarUpdateStart = profiler->Start();
					
					this->m_Loader->LoadImageToCache(this->m_Frame);

					// Load all the frames from the frame we clicked on
					if (this->m_Loader->m_UseCache)
					{
						this->m_Loader->m_Workers.emplace_back(&Core::Loader::LoadSequenceToCache, this->m_Loader, this->m_Frame, 0);
					}

					// this->m_Update = false;
					
					const auto playbarUpdateEnd = profiler->End();
					
					profiler->Time("Playbar Update Time", playbarUpdateStart, playbarUpdateEnd);
				}
			}
			
			const auto cacheUpdateStart = profiler->Start();

			// Update cache indices
			for (uint32_t i = this->m_Range.x; i < this->m_Range.y; i++)
			{
				const Core::Image* tmpImage = this->m_Loader->GetImage(i);

				this->m_CachedIndices[i] = tmpImage->m_CacheIndex > 0 ? 1 : 0;
			}

			const auto cacheUpdateEnd = profiler->End();

			profiler->Time("Playbar Cache Indices Update Time", cacheUpdateStart, cacheUpdateEnd);
		}
	}

	void ImPlaybar::SetRange(const ImVec2& newRange) noexcept
	{
		this->Pause();

		this->m_Frame = 0;

		this->m_Range = newRange;

		this->m_Update = true;

		this->m_CachedIndices.resize(newRange.y);
		for (uint32_t i = 0; i < newRange.y; i++) this->m_CachedIndices[i] = false;
	}
 
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

			const ImVec2 timelineP0 = ImGui::GetCursorScreenPos();
			const ImVec2 timelineSize = ImGui::GetContentRegionAvail() - ImVec2(200.0f, 0.0f);
			const ImVec2 timelineP1 = ImVec2(timelineP0.x + timelineSize.x, timelineP0.y + timelineSize.y);

			ImGui::InvisibleButton("Playbar", timelineSize);

			drawList->AddRectFilled(timelineP0, timelineP1, DARKGRAY);

			drawList->PushClipRect(timelineP0, timelineP1, true);

			const float step = timelineSize.x / (this->m_Range.y);

			// Separating lines and frame number indicators
			for (float x = this->m_Range.x; x < (this->m_Range.y); x++)
			{
				float thickness = 1.0f;
				float height = (timelineP0.y + 10.0f);
				
				if (fmodf(x + 1.0f, 24.0f) == 0.0f || x == 0.0f)
				{
					thickness = 2.0f;
					height = timelineP1.y;

					char frameIndexText[16];
					sprintf(frameIndexText, "%d", (int)x + 1);

					drawList->AddText(nullptr, 15.0f, ImVec2(timelineP0.x + (step * x) + 8.0f, timelineP0.y + 12.5f), LIGHTGRAY, frameIndexText);
				}

				drawList->AddLine(ImVec2(timelineP0.x + (step * x) + 2.0f, timelineP0.y), ImVec2(timelineP0.x + (step * x) + 2.0f, height), LIGHTGRAY, thickness);

				if (this->m_CachedIndices[static_cast<int>(x)])
				{
					drawList->AddQuadFilled(ImVec2(timelineP0.x + (step * x) + 2.0f, timelineP1.y - 10.0f),
											ImVec2(timelineP0.x + (step * (x + 1.0f)) + 2.0f, timelineP1.y - 10.0f),
											ImVec2(timelineP0.x + (step * (x + 1.0f)) + 2.0f, timelineP1.y),
											ImVec2(timelineP0.x + (step * x) + 2.0f, timelineP1.y), LIGHTGREEN);
				}
			}	

			// Cursor
			const float cursorOffset = 1.5f;

			drawList->AddLine(ImVec2(timelineP0.x + (step * (this->m_Frame)) + cursorOffset, timelineP0.y), ImVec2(timelineP0.x + (step * (this->m_Frame)) + cursorOffset, timelineP1.y), LIGHTBLUE, cursorOffset);
			drawList->AddTriangleFilled(ImVec2(timelineP0.x + (step * (this->m_Frame)) - 8.0f + cursorOffset, timelineP0.y),
										ImVec2(timelineP0.x + (step * (this->m_Frame)) + 8.0f + cursorOffset, timelineP0.y),
										ImVec2(timelineP0.x + (step * (this->m_Frame)) + cursorOffset, timelineP0.y + 6.0f), LIGHTBLUE);

			drawList->PopClipRect();

			// Click to set frame freely
			if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				const float x_position = ImGui::GetIO().MouseClickedPos->x - ImGui::GetCursorScreenPos().x - ImGui::GetScrollX();
				this->m_Frame = floor(x_position / step);
				this->m_Scrolling = x_position;
				this->m_Play = false;
				this->m_Update = true;
			}

			// Dragging and play frames dragged
			if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			{
				this->m_Scrolling += ImGui::GetIO().MouseDelta.x;
				this->m_Scrolling = this->m_Scrolling < 0 ? 0.0f : this->m_Scrolling;
				this->m_Scrolling = this->m_Scrolling > (this->m_Range.y - 1.0f) * step ? (this->m_Range.y - 2.0f) * step : this->m_Scrolling;
				this->m_Frame = this->m_Scrolling / step;
				this->m_Play = false;
				this->m_Update = true;
			}	

			// Buttons
			constexpr float buttonsWidth = 50.0f;
			constexpr float buttonsHeight = 25.0f;
			const ImVec2 buttonsP0 = ImVec2(timelineP1.x, timelineP0.y);
			const ImVec2 buttonsP1 = ImVec2(timelineP1.x + 200.0f, timelineP0.y + buttonsHeight);
			ImGui::PushClipRect(buttonsP0, buttonsP1, true);

			const ImVec2 mousePos = ImGui::GetMousePos();

			// Left button
			constexpr float leftOffset1 = 0.0f;
			constexpr float leftOffset2 = 50.0f;
			const ImVec2 leftP0 = ImVec2(timelineP1.x + leftOffset1, timelineP0.y);
			const ImVec2 leftP1 = ImVec2(timelineP1.x + leftOffset2, timelineP0.y + buttonsHeight);

			drawList->AddTriangleFilled(ImVec2(leftP0.x + 10.0f, (leftP0.y + leftP1.y) / 2.0f), 
										ImVec2(leftP0.x + 25.0f, leftP0.y + 5.0f), 
										ImVec2(leftP0.x + 25.0f, leftP0.y + 20.0f), LIGHTGRAY);
			drawList->AddTriangleFilled(ImVec2(leftP0.x + 25.0f, (leftP0.y + leftP1.y) / 2.0f), 
										ImVec2(leftP0.x + 40.0f, leftP0.y + 5.0f), 
										ImVec2(leftP0.x + 40.0f, leftP0.y + 20.0f), LIGHTGRAY);

			if (Hover(leftP0, leftP1, mousePos))
			{
				drawList->AddRectFilled(leftP0, leftP1, HOVERGRAY);

				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) 
				{
					this->Pause();
					this->m_Frame = this->m_Range.x;
					this->m_Update = true;
				}
			}

			// Play button
			constexpr float playOffset1 = 50.0f;
			constexpr float playOffset2 = 100.0f;
			const ImVec2 playP0 = ImVec2(timelineP1.x + playOffset1, timelineP0.y);
			const ImVec2 playP1 = ImVec2(timelineP1.x + playOffset2, timelineP0.y + buttonsHeight);

			const ImColor playButtonColor = this->m_Play ? LIGHTBLUE : LIGHTGRAY;

			drawList->AddTriangleFilled(ImVec2(playP0.x + 17.5f, leftP0.y + 5.0f),
										ImVec2(playP0.x + 32.5f, (playP0.y + playP1.y) / 2.0f),
										ImVec2(playP0.x + 17.5f, leftP0.y + 20.0f), playButtonColor);
			
			if (Hover(playP0, playP1, mousePos))
			{
				drawList->AddRectFilled(playP0, playP1, HOVERGRAY);

				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) this->Play();
			}

			// Stop button
			constexpr float stopOffset1 = 100.0f;
			constexpr float stopOffset2 = 150.0f;
			const ImVec2 stopP0 = ImVec2(timelineP1.x + stopOffset1, timelineP0.y);
			const ImVec2 stopP1 = ImVec2(timelineP1.x + stopOffset2, timelineP0.y + buttonsHeight);

			drawList->AddRectFilled(ImVec2(stopP0.x + 17.5f, stopP0.y + 5.0f),
									ImVec2(stopP0.x + 32.5f, stopP0.y + 20.0f), LIGHTGRAY);

			if (Hover(stopP0, stopP1, mousePos))
			{
				drawList->AddRectFilled(stopP0, stopP1, HOVERGRAY);
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) this->Pause();
			}

			// Right button
			constexpr float rightOffset1 = 150.0f;
			constexpr float rightOffset2 = 200.0f;
			const ImVec2 rightP0 = ImVec2(timelineP1.x + rightOffset1, timelineP0.y);
			const ImVec2 rightP1 = ImVec2(timelineP1.x + rightOffset2, timelineP0.y + buttonsHeight);

			drawList->AddTriangleFilled(ImVec2(rightP0.x + 10.0f, rightP0.y + 5.0f), 
										ImVec2(rightP0.x + 25.0f, (rightP0.y + rightP1.y) / 2.0f),
										ImVec2(rightP0.x + 10.0f, rightP0.y + 20.0f), LIGHTGRAY);
			drawList->AddTriangleFilled(ImVec2(rightP0.x + 25.0f, rightP0.y + 5.0f), 
										ImVec2(rightP0.x + 40.0f, (rightP0.y + rightP1.y) / 2.0f),
										ImVec2(rightP0.x + 25.0f, rightP0.y + 20.0f), LIGHTGRAY);

			if(Hover(rightP0, rightP1, mousePos))
			{
				drawList->AddRectFilled(rightP0, rightP1, HOVERGRAY);

				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				{
					this->Pause();
					this->m_Update = true;
					this->m_Frame = this->m_Range.y - 1;
				}
			}

			ImGui::PopClipRect();

			// Frame number and framerate
			const ImVec2 frameInfosP0 = ImVec2(timelineP1.x, buttonsP1.y);
			const ImVec2 frameInfosP1 = ImVec2(timelineP1.x + 200.0f, timelineP1.y);
			
			ImGui::PushClipRect(frameInfosP0, frameInfosP1, true);

			char frameNumberText[16];
			sprintf(frameNumberText, "%d", (this->m_Frame + 1));

			drawList->AddText(nullptr, 20.0f, ImVec2(frameInfosP0.x + 5.0f, frameInfosP0.y + 10.0f), LIGHTGRAY, frameNumberText);

			ImGui::PopClipRect();
			
			static int framerateTmp = this->m_FrameRate;
			
			// ImGui::SetCursorPos(ImVec2(frameInfosP0.x + 5.0f, frameInfosP0.y + 10.0f));
			ImGui::SameLine();
			ImGui::SetCursorPos(ImGui::GetCursorPos() - ImVec2(-50.0f, -37.5f));
			ImGui::SetNextItemWidth(100.0f);
			ImGui::InputInt("", &framerateTmp, 0);
			if (ImGui::IsItemEdited()) this->m_FrameRate = static_cast<uint8_t>(framerateTmp);
		}
		ImGui::End();
	}
} // End namespace Interface