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

		Core::Loader* m_Loader = nullptr;

		ImVec2 m_Range;

		float m_Scrolling = 0.0f;

		uint32_t m_Frame = 0;
		
		uint8_t m_FrameRate = 24;
		
		bool m_Play = false;
		bool m_Update = true;

		ImPlaybar(Core::Loader* loader, ImVec2 range) : 
			m_Loader(loader),
			m_Range(range)
		{
			this->m_CachedIndices.resize(range.y);
			for (uint32_t i = 0; i < range.y; i++) this->m_CachedIndices[i] = false;
		}

		void Play() noexcept;
		void Pause() noexcept;
		void Update(Profiler* profiler) noexcept;
		void SetRange(const ImVec2& newRange) noexcept;

		void Draw() noexcept;
	};

	OPENVIEWER_FORCEINLINE bool Hover(ImVec2 min, ImVec2 max, ImVec2 pos) noexcept
	{
		if (pos.x > min.x && pos.y > min.y && pos.x < max.x && pos.y < max.y) return true;
		else return false;
	}
} // End namespace Interface