// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#pragma once

#include "imgui.h"

#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui_internal.h"

#include "utils/decl.h"
#include "core/loader.h"

#include "cstdint"
#include "math.h"
#include <vector>

#define LIGHTGRAY IM_COL32(128, 128, 128, 255)
#define DARKGRAY IM_COL32(10, 10, 10, 40)
#define LIGHTBLUE IM_COL32(120, 150, 200, 255)
#define HOVERGRAY IM_COL32(128, 128, 128, 128)
#define LIGHTGREEN IM_COL32(0, 255, 0, 255)

namespace Interface
{
	struct ImPlaybar
	{
		std::vector<uint8_t> m_CachedIndices;

		std::thread m_PlayerThread;

		std::mutex m_Mutex;

		std::condition_variable m_CV;

		Core::Loader* m_Loader = nullptr;

		ImVec2 m_Range;

		float m_Scrolling = 0.0f;

		uint32_t m_Frame = 0;
		
		uint8_t m_FrameRate = 24;
		
		bool m_Play = false;
		bool m_Pause = false;
		bool m_Update = true;
		bool m_Release = false;
		bool m_IsDragging = false;

		ImPlaybar(Core::Loader* loader, ImVec2 range) : 
			m_Loader(loader),
			m_Range(range)
		{
			this->m_CachedIndices.resize(range.y);
			for (uint32_t i = 0; i < range.y; i++) this->m_CachedIndices[i] = false;

			this->m_PlayerThread = std::thread(&ImPlaybar::BackgroundTimeUpdate, this);
		}

		void Play() noexcept;

		void Pause() noexcept;

		void GoPreviousFrame() noexcept;

		void GoNextFrame() noexcept;

		void GoFirstFrame() noexcept;

		void GoLastFrame() noexcept;

		bool IsPlaying() const noexcept { return this->m_Play; }

		void Update(Profiler* profiler) noexcept;

		void SetRange(const ImVec2& newRange) noexcept;

		void BackgroundTimeUpdate() noexcept;

		void NeedUpdate(const bool need = true) noexcept;

		void Release() noexcept;

		void Draw() noexcept;
	};

	OPENVIEWER_FORCEINLINE bool Hover(ImVec2 min, ImVec2 max, ImVec2 pos) noexcept
	{
		if (pos.x > min.x && pos.y > min.y && pos.x < max.x && pos.y < max.y) return true;
		else return false;
	}
} // End namespace Interface