// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "implaybar.h"

namespace Interface
{
	void ImPlaybar::Initialize(Core::Loader* loader, ImVec2 range, uint8_t id) noexcept
	{
		this->m_Loader = loader;

		this->m_PlaybarID = id;

		this->m_Range = range;

		this->m_CachedIndices.resize(range.y);
		for (uint32_t i = 0; i < range.y; i++) this->m_CachedIndices[i] = false;

		this->m_PlayerThread = std::thread(&ImPlaybar::BackgroundTimeUpdate, this);
	}

	void ImPlaybar::Play() noexcept 
	{ 
		std::unique_lock<std::mutex> playLock(this->m_Mutex);

		this->m_Play = true; 	
		this->m_Pause = false;

		playLock.unlock();

		this->m_CV.notify_all();
	}

	void ImPlaybar::Pause() noexcept 
	{ 
		std::unique_lock<std::mutex> playLock(this->m_Mutex);

		this->m_Play = false; 
		this->m_Pause = true;	

		playLock.unlock();

		this->m_CV.notify_all();
	}

	void ImPlaybar::GoPreviousFrame() noexcept
	{
		this->Pause();

		std::unique_lock<std::mutex> playLock(this->m_Mutex);

		this->m_Frame = this->m_Frame - 1 <= static_cast<uint32_t>(this->m_Range.x) || this->m_Frame == 0 ? 
						static_cast<uint32_t>(this->m_Range.y - 1) : 
						this->m_Frame - 1;
		
		playLock.unlock();
		
		this->NeedUpdate();	
	}

	void ImPlaybar::GoNextFrame() noexcept
	{
		this->Pause();

		std::unique_lock<std::mutex> playLock(this->m_Mutex);

		this->m_Frame = this->m_Frame + 1 >= static_cast<uint32_t>(this->m_Range.y) ? 
						static_cast<uint32_t>(this->m_Range.x) : 
						this->m_Frame + 1; 

		playLock.unlock();

		this->NeedUpdate();
	}

	void ImPlaybar::GoFirstFrame() noexcept
	{
		this->Pause();
	
		std::unique_lock<std::mutex> playLock(this->m_Mutex);

		this->m_Frame = this->m_Range.x;

		playLock.unlock();

		this->NeedUpdate();	
	}

	void ImPlaybar::GoLastFrame() noexcept
	{
		this->Pause();
	
		std::unique_lock<std::mutex> playLock(this->m_Mutex);
	
		this->m_Frame = this->m_Range.y - 1;
		
		playLock.unlock();
		
		this->NeedUpdate();	
	}

	void ImPlaybar::Update() noexcept
	{
		if (this->m_Loader->m_HasBeenInitialized)
		{
			// Update the range of the loader to match the one of the playbar
			this->m_Loader->m_Range = this->m_Range;

			if (!this->m_Play)
			{
				if (this->m_Update)
				{
					this->m_Loader->LoadImageToCache(this->m_MediaId, this->m_Frame);

					// Load all the frames from the frame we clicked on
					if (this->m_Loader->m_CacheMode > 0 && !this->m_IsDragging)
					{
						this->m_Loader->m_Workers.emplace_back(&Core::Loader::LoadSequenceToCache, this->m_Loader, this->m_MediaId, this->m_Frame, 0);
					}

					// this->NeedUpdate(false);
				}
			}

			// Update cache indices
			for (uint32_t i = this->m_Range.x; i < this->m_Range.y; i++)
			{
				const Core::Image* tmpImage = this->m_Loader->GetImage(this->m_MediaId, i);

				this->m_CachedIndices[i] = tmpImage->m_CacheIndex > 0 ? 1 : 0;
			}
		}
	}

	void ImPlaybar::SetRange(const ImVec2& newRange) noexcept
	{
		this->Pause();

		this->GoFirstFrame();

		this->m_Range = newRange;

		this->NeedUpdate();

		this->m_CachedIndices.resize(newRange.y);
		for (uint32_t i = 0; i < newRange.y; i++) this->m_CachedIndices[i] = false;
	}
 
	void ImPlaybar::Draw() noexcept
	{
		bool p_open = true;

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | 
										ImGuiWindowFlags_NoCollapse;

		constexpr float playbarHeight = 80.0f;

		char playbarName[32];
		Utils::Str::Format(playbarName, "Playbar %d", this->m_PlaybarID);

		ImGui::SetNextWindowSizeConstraints(ImVec2(400.0f, playbarHeight), ImVec2(8192.0f, playbarHeight));

		ImGui::Begin(playbarName, &p_open, window_flags);
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
					drawList->AddQuadFilled(ImVec2(timelineP0.x + (step * x) + 2.0f, timelineP1.y - 2.5f),
											ImVec2(timelineP0.x + (step * (x + 1.0f)) + 2.0f, timelineP1.y - 2.5f),
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
				this->Pause();
				this->NeedUpdate();
			}

			// Dragging and play frames dragged
			if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			{
				this->m_IsDragging = true;
				this->m_Scrolling += ImGui::GetIO().MouseDelta.x;
				this->m_Scrolling = this->m_Scrolling < 0 ? 0.0f : this->m_Scrolling;
				this->m_Scrolling = this->m_Scrolling > (this->m_Range.y - 1.0f) * step ? (this->m_Range.y - 1.0f) * step : this->m_Scrolling;
				this->m_Frame = this->m_Scrolling / step;
				this->Pause();
				this->NeedUpdate();
			}	
			
			if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
				this->m_IsDragging = false;
				this->m_Update = true;
			}


			// Buttons
			constexpr float buttonsWidth = 40.0f;
			constexpr float buttonsHeight = 25.0f;

			const ImVec2 buttonsP0 = ImVec2(timelineP1.x + 5.0f, timelineP0.y);
			const ImVec2 buttonsP1 = ImVec2(timelineP1.x + 205.0f, timelineP0.y + buttonsHeight);
			
			ImGui::PushClipRect(buttonsP0, buttonsP1, true);

			const ImVec2 mousePos = ImGui::GetMousePos();

			// Go Start Frame / Leftmost button
			constexpr float leftOffset1 = 5.0f;
			constexpr float leftOffset2 = leftOffset1 + buttonsWidth;
			
			const ImVec2 leftP0 = ImVec2(timelineP1.x + leftOffset1, timelineP0.y);
			const ImVec2 leftP1 = ImVec2(timelineP1.x + leftOffset2, timelineP0.y + buttonsHeight);

			drawList->AddTriangleFilled(ImVec2(leftP0.x + 7.5f, (leftP0.y + leftP1.y) / 2.0f), 
										ImVec2(leftP0.x + 22.5f, leftP0.y + 5.0f), 
										ImVec2(leftP0.x + 22.5f, leftP0.y + 20.0f), LIGHTGRAY);
			drawList->AddTriangleFilled(ImVec2(leftP0.x + 17.5f, (leftP0.y + leftP1.y) / 2.0f), 
										ImVec2(leftP0.x + 32.5f, leftP0.y + 5.0f), 
										ImVec2(leftP0.x + 32.5f, leftP0.y + 20.0f), LIGHTGRAY);

			if (Hover(leftP0, leftP1, mousePos))
			{
				drawList->AddRectFilled(leftP0, leftP1, HOVERGRAY);

				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) 
				{
					this->GoFirstFrame();
				}
			}

			// Go Previous Frame / Button
			constexpr float left2Offset1 = leftOffset2;
			constexpr float left2Offset2 = left2Offset1 + buttonsWidth;

			const ImVec2 left2P0 = ImVec2(timelineP1.x + left2Offset1, timelineP0.y);
			const ImVec2 left2P1 = ImVec2(timelineP1.x + left2Offset2, timelineP0.y + buttonsHeight);

			drawList->AddTriangleFilled(ImVec2(left2P0.x + 12.5f, (left2P0.y + left2P1.y) / 2.0f),
										ImVec2(left2P0.x + 27.5f, left2P0.y + 5.0f),
										ImVec2(left2P0.x + 27.5f, left2P0.y + 20.0f), LIGHTGRAY);

			if (Hover(left2P0, left2P1, mousePos))
			{
				drawList->AddRectFilled(left2P0, left2P1, HOVERGRAY);

				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) 
				{
					this->GoPreviousFrame();
				}
			}

			// Play / Stop button
			constexpr float playOffset1 = left2Offset2;
			constexpr float playOffset2 = playOffset1 + buttonsWidth;
			const ImVec2 playP0 = ImVec2(timelineP1.x + playOffset1, timelineP0.y);
			const ImVec2 playP1 = ImVec2(timelineP1.x + playOffset2, timelineP0.y + buttonsHeight);

			const ImColor playButtonColor = this->m_Play ? LIGHTBLUE : LIGHTGRAY;

			if (this->m_Play)
			{
				drawList->AddRectFilled(ImVec2(playP0.x + 12.5f, playP0.y + 5.0f),
									    ImVec2(playP0.x + 27.5f, playP0.y + 20.0f), LIGHTGRAY);
			}
			else
			{
				drawList->AddTriangleFilled(ImVec2(playP0.x + 12.5f, leftP0.y + 5.0f),
											ImVec2(playP0.x + 27.5f, (playP0.y + playP1.y) / 2.0f),
											ImVec2(playP0.x + 12.5f, leftP0.y + 20.0f), playButtonColor);
			}

			if (Hover(playP0, playP1, mousePos))
			{
				drawList->AddRectFilled(playP0, playP1, HOVERGRAY);

				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				{
					if (this->m_Play) this->Pause(); 
					else this->Play();
				}
					
			}

			// Go Next Frame // Button
			constexpr float right2Offset1 = playOffset2;
			constexpr float right2Offset2 = right2Offset1 + buttonsWidth;
			const ImVec2 right2P0 = ImVec2(timelineP1.x + right2Offset1, timelineP0.y);
			const ImVec2 right2P1 = ImVec2(timelineP1.x + right2Offset2, timelineP0.y + buttonsHeight);

			drawList->AddTriangleFilled(ImVec2(right2P0.x + 12.5f, right2P0.y + 5.0f),
										ImVec2(right2P0.x + 27.5f, (right2P0.y + right2P1.y) / 2.0f),
										ImVec2(right2P0.x + 12.5f, right2P0.y + 20.0f), LIGHTGRAY);

			if (Hover(right2P0, right2P1, mousePos))
			{
				drawList->AddRectFilled(right2P0, right2P1, HOVERGRAY);
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				{
					this->GoNextFrame();
				}
			}

			// Go Last Frame / Right button
			constexpr float rightOffset1 = right2Offset2;
			constexpr float rightOffset2 = rightOffset1 + buttonsWidth;
			const ImVec2 rightP0 = ImVec2(timelineP1.x + rightOffset1, timelineP0.y);
			const ImVec2 rightP1 = ImVec2(timelineP1.x + rightOffset2, timelineP0.y + buttonsHeight);

			drawList->AddTriangleFilled(ImVec2(rightP0.x + 7.5f, rightP0.y + 5.0f), 
										ImVec2(rightP0.x + 22.5f, (rightP0.y + rightP1.y) / 2.0f), 
										ImVec2(rightP0.x + 7.5f, rightP0.y + 20.0f), LIGHTGRAY);
			drawList->AddTriangleFilled(ImVec2(rightP0.x + 17.5f, rightP0.y + 5.0f), 
										ImVec2(rightP0.x + 32.5f, (rightP0.y + rightP1.y) / 2.0f), 
										ImVec2(rightP0.x + 17.5f, rightP0.y + 20.0f), LIGHTGRAY);

			if(Hover(rightP0, rightP1, mousePos))
			{
				drawList->AddRectFilled(rightP0, rightP1, HOVERGRAY);

				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				{
					this->GoLastFrame();
				}
			}

			ImGui::PopClipRect();

			// Frame number and framerate
			const ImVec2 frameInfosP0 = ImVec2(timelineP1.x, buttonsP1.y);
			const ImVec2 frameInfosP1 = ImVec2(timelineP1.x + 200.0f, timelineP1.y);
			
			ImGui::PushClipRect(frameInfosP0, frameInfosP1, true);

			const uint32_t frame = (this->m_Frame) % this->m_FrameRate + 1;
			const uint32_t seconds = static_cast<uint32_t>(std::floor(static_cast<float>(this->m_Frame) / static_cast<float>(this->m_FrameRate))) % 60;
			const uint32_t minutes = std::floor(static_cast<float>(seconds) / 60.0f);
			const uint32_t hours = std::floor(static_cast<float>(minutes) / 60.0f);

			char frameNumberText[32];
			Utils::Str::Format(frameNumberText, "%02d:%02d:%02d:%02d", hours, minutes, seconds, frame);
			drawList->AddText(nullptr, 20.0f, ImVec2(frameInfosP0.x + 10.0f, frameInfosP0.y + 10.0f), WHITE, frameNumberText);

			char framerateText[16];
			Utils::Str::Format(framerateText, "%.1f", this->m_RealFramerate);
			drawList->AddText(nullptr, 20.0f, ImVec2(frameInfosP0.x + 170.0f, frameInfosP0.y + 10.0f), WHITE, framerateText);

			ImGui::PopClipRect();
			
			static int framerateTmp = this->m_FrameRate;
		}

		ImGui::End();
	}

	void ImPlaybar::BackgroundTimeUpdate() noexcept
	{
		uint32_t sleepTime = static_cast<uint32_t>(1.0f / static_cast<float>(this->m_FrameRate) * 1000.0f);
		
		while (true)
		{
			std::unique_lock<std::mutex> bgUpdateLock(this->m_Mutex);

			this->m_CV.wait_for(bgUpdateLock, std::chrono::milliseconds(sleepTime), [this] { return this->m_Release || this->m_Pause; });

			sleepTime = static_cast<uint32_t>(1.0f / static_cast<float>(this->m_FrameRate) * 1000.0f);

			if (this->m_Release)
			{
				bgUpdateLock.unlock();
				break;
			}

			if (this->m_Pause)
			{
				bgUpdateLock.unlock();
			}
			else if (this->m_Play)
			{
				const auto updateStart = Profiler::StaticStart();

				this->m_Frame = fmodf(this->m_Frame + 1, (this->m_Range.y));
				this->m_Update = true;

				// Notify the background cache loader that we need to store some frames in the cache
				if (this->m_Loader->m_CacheMode > 0)
				{
					// Urgent load in case of fast scrolling while cache is activated
					if (!this->m_CachedIndices[this->m_Frame]) this->m_Loader->LoadImageToCache(this->m_MediaId, this->m_Frame);

					const uint32_t offset = this->m_Loader->m_Cache->m_Size - this->m_Loader->m_BgLoadChunkSize * 2;
					const uint32_t frameIdx = static_cast<uint32_t>(fmodf((this->m_Frame + offset), (this->m_Range.y - 1)));

					if (!this->m_CachedIndices[frameIdx])
					{
						std::unique_lock<std::mutex> playbarLock(this->m_Loader->m_Mutex);

						this->m_Loader->m_NeedBgLoad = true;
						this->m_Loader->m_BgLoadFrameIndex = frameIdx;

						playbarLock.unlock();
					}
				}
				else
				{
					this->m_Loader->LoadImageToCache(this->m_MediaId, this->m_Frame);
				}

				const auto updateEnd = Profiler::StaticEnd();

				const float loadTime = Profiler::StaticTime(updateStart, updateEnd);

				this->m_RealFramerate = 1000.0f / std::max(loadTime, static_cast<float>(sleepTime));

				sleepTime = sleepTime - std::min(static_cast<uint32_t>(loadTime), sleepTime) + 1;
				
				bgUpdateLock.unlock();
			}
		}
	}

	void ImPlaybar::NeedUpdate(const bool need) noexcept
	{
		std::unique_lock<std::mutex> lock(this->m_Mutex);
		this->m_Update = need;
		lock.unlock();

		m_CV.notify_all();
	}

	void ImPlaybar::Release() noexcept
	{
		std::unique_lock<std::mutex> lock(this->m_Mutex);
		this->m_Release = true;
		lock.unlock();

		// Make sure the thread has finished working
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

		// Join it
		if (this->m_PlayerThread.joinable()) this->m_PlayerThread.join();
	}
} // End namespace Interface