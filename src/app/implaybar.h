// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#pragma once

#include "imgui.h"

#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui_internal.h"

#include "utils/decl.h"

#include "cstdint"
#include "math.h"
#include <vector>

#define LIGHTGRAY IM_COL32(128, 128, 128, 255)
#define DARKGRAY IM_COL32(10, 10, 10, 40)
#define LIGHTBLUE IM_COL32(120, 150, 200, 255)
#define HOVERGRAY IM_COL32(128, 128, 128, 128)

namespace Interface
{
	struct ImPlaybar
	{
		ImVec2 m_Range;

		float m_Scrolling = 0.0f;

		uint16_t m_Frame = 0;
		
		uint8_t m_FrameRate = 24;
		
		bool m_Play = false;
		bool m_Update = false;

		ImPlaybar(ImVec2 range) : 
			m_Range(range)
		{}

		void Play() noexcept;
		void Pause() noexcept;
		void Update() noexcept;

		void Draw() noexcept;
	};

	OPENVIEWER_FORCEINLINE bool Hover(ImVec2 min, ImVec2 max, ImVec2 pos) noexcept
	{
		if (pos.x > min.x && pos.y > min.y && pos.x < max.x && pos.y < max.y) return true;
		else return false;
	}
} // End namespace Interface