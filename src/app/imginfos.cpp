// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "imginfos.h"

namespace Interface
{
    void ImageInfo::Draw(const Core::Image& currentImage, bool& showWindow) const noexcept
    {
        if (showWindow)
        {
            ImGui::Begin("Image Infos", &showWindow);
            {
                ImGui::Text("Resolution : %dx%d", currentImage.m_Xres, currentImage.m_Yres);
                ImGui::Text("Channels : %d", currentImage.m_Channels);
            }   
            ImGui::End();
        }
    }

    void PixelInfo::Draw(Core::Loader* loader, const Core::Image& currentImage, Display* currentDisplay, bool& showWindow) const noexcept
    {
        if (showWindow)
        {
            ImGui::Begin("Pixel Info", &showWindow);
            {
                const uint16_t X = static_cast<uint16_t>(currentDisplay->m_HoverCoordinates.x);
                const uint16_t Y = static_cast<uint16_t>(currentDisplay->m_HoverCoordinates.y);

                ImGui::Text("X : %d", X); 
                ImGui::SameLine();
                ImGui::Text("Y : %d", Y);

                ImGui::Separator();

                ImGui::Text("Corrected Color");
                const ImVec4 currentColorCorrected = ImClamp(currentDisplay->GetPixel(X, Y), 
                                                             ImVec4(0.0f, 0.0f, 0.0f, 0.0f), 
                                                             ImVec4(100.0f, 100.0f, 100.0f, 100.0f));

                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "R : %f", currentColorCorrected.x);
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "G : %f", currentColorCorrected.y);
                ImGui::TextColored(ImVec4(0.0f, 0.0f, 1.0f, 1.0f), "B : %f", currentColorCorrected.z);
                ImGui::Text("A : %f", currentColorCorrected.w);

                ImGui::Text("Color :");
                ImGui::SameLine();
                ImGui::ColorButton("correctedColor", currentColorCorrected, ImGuiColorEditFlags_NoTooltip, ImVec2(30.0f, 30.0f));

                ImGui::Separator();
                
                ImGui::Text("Raw Color");

                void* currentImageBuffer = loader->m_Cache->m_Items[currentImage.m_CacheIndex].m_DataPtr;
                
                ImVec4 currentColorRaw = currentColorCorrected;

                if (currentImage.m_CacheIndex > 0)
                {
                    currentColorRaw = ImClamp(currentImage.GetPixel(X, Y, currentImageBuffer), 
                                                        ImVec4(0.0f, 0.0f, 0.0f, 0.0f), 
                                                        ImVec4(100.0f, 100.0f, 100.0f, 100.0f));
                }

                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "R : %f", currentColorRaw.x);
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "G : %f", currentColorRaw.y);
                ImGui::TextColored(ImVec4(0.0f, 0.0f, 1.0f, 1.0f), "B : %f", currentColorRaw.z);
                ImGui::Text("A : %f", currentColorRaw.w);

                ImGui::Text("Color :");
                ImGui::AlignTextToFramePadding();
                ImGui::SameLine();
                ImGui::ColorButton("rawColor", currentColorRaw, ImGuiColorEditFlags_NoTooltip, ImVec2(30.0f, 30.0f));
            }
            ImGui::End();
        }
    }
}