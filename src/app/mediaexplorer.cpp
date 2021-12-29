// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "mediaexplorer.h"

namespace Interface
{
    MediaExplorer::MediaExplorer(Core::Loader* loader, Logger* logger)
    {
        this->m_Loader = loader;
        this->m_Logger = logger;
    }

    void MediaExplorer::Draw(bool& showWindow) noexcept
    {
        if (showWindow)
        {
            ImGui::Begin("Media Explorer", &showWindow);
            {
                static bool selected = false;

                for (uint32_t i = 0; i < this->m_Loader->m_MediaCount; i++)
                {
                    const std::string mediaPath = this->m_Loader->m_Medias[i].m_Images[0].m_Path;

                    const bool isSequence = this->m_Loader->m_Medias[i].m_Range.y > 0;

                    char selectableLabel[8192];
                    
                    if (isSequence) sprintf_s(selectableLabel, "%s (sequence [%d images])", mediaPath.c_str(), 
                                                                                            static_cast<uint32_t>(this->m_Loader->m_Medias[i].Size()));
                    else sprintf_s(selectableLabel, "%s (single image)", mediaPath.c_str());

                    ImGui::Selectable(selectableLabel, selected);

                    if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
                    {
                        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                        {
                            if (this->m_Loader->m_Medias[i].m_IsActive) continue;
                            else
                            {
                                this->m_Loader->m_Medias[i].SetActive();

                                this->m_Logger->Log(LogLevel_Diagnostic, "[MEDIA] : Media %s [ID : %d] is now active", mediaPath.c_str(), this->m_Loader->m_Medias[i].m_ID);

                                for (uint32_t j = 0; j < this->m_Loader->m_MediaCount; j++) 
                                {
                                    if (this->m_Loader->m_Medias[j].m_ID == this->m_Loader->m_Medias[i].m_ID) continue;
                                    else this->m_Loader->m_Medias[j].SetInactive();
                                }

                                this->m_CurrentMediaRange = this->m_Loader->m_Medias[i].m_TimelineRange;

                                this->m_CurrentMediaChanged = true;
                            }
                        }
                    }
                }
            }
            ImGui::End();
        }
    }
}