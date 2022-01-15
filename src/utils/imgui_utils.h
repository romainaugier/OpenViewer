// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 Romain Augier
// All rights reserved.

#pragma once

#include "imgui.h"
#include "decl.h"

#define IM_ID(id, stmt) do { \
            ImGui::PushID(id); \
            stmt; \
            ImGui::PopID(); \
        } while (0)

namespace ImGui
{
    OV_STATIC_FUNC void SameLineWidget(const float widgetWidth) noexcept
    {
        SameLine();

        const ImVec2 leftSpace = ImGui::GetContentRegionAvail();

        Dummy(ImVec2(leftSpace.x - 100.0f, 1.0f));
        SameLine();

        SetNextItemWidth(100.0f);
}
} // End namespace ImGui