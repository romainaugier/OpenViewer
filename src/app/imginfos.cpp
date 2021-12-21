// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "imginfos.h"

namespace Interface
{
    void ImageInfo::Draw(const Core::Image& currentImage, bool* showWindow) const noexcept
    {
        ImGui::Begin("Image Infos", showWindow);
        {
            ImGui::Text("Resolution : %dx%d", currentImage.m_Xres, currentImage.m_Yres);
            ImGui::Text("Channels : %d", currentImage.m_Channels);
        }   
        ImGui::End();
    }
}