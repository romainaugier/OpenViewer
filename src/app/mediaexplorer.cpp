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

    void MediaExplorer::Draw(Application* app, bool& showWindow) noexcept
    {
        if (showWindow)
        {
            ImGui::Begin("Media Explorer", &showWindow);
            {
                static bool selected = false;

                for (uint32_t i = 0; i < this->m_Loader->m_MediaCount; i++)
                {
                    const std::string mediaPath = this->m_Loader->m_Medias[i].m_Images[0].m_Path;

                    const bool isSequence = this->m_Loader->m_Medias[i].m_Range.y > 1;

                    char selectableLabel[8192];
                    
                    if (isSequence) Utils::Str::Format(selectableLabel, "%s (%d images)", mediaPath.c_str(), 
                                                                                          static_cast<uint32_t>(this->m_Loader->m_Medias[i].Size()));
                    else Utils::Str::Format(selectableLabel, "%s (1 image)", mediaPath.c_str());

                    ImGui::Selectable(selectableLabel, selected);

                    if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
                    {
                        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                        {
                            if (this->m_Loader->m_Medias[i].m_IsActive) continue;
                            else
                            {
                                for (uint32_t j = 0; j < this->m_Loader->m_MediaCount; j++) 
                                {
                                    this->m_Loader->SetMediaInactive(j);
                                }

                                this->m_Logger->Log(LogLevel_Diagnostic, "[MEDIA] : Media %s [ID : %d] is now active", mediaPath.c_str(), this->m_Loader->m_Medias[i].m_ID);

                                this->m_Loader->SetMediaActive(i);
                                this->m_Loader->LoadImageToCache(0);

                                this->m_CurrentMediaRange = this->m_Loader->m_Medias[i].m_TimelineRange;

                                this->m_CurrentMediaChanged = true;
                            }

                            // If no display is active, create one
                            if (app->m_DisplayCount == 0)
                            {
                                this->m_Loader->LoadImageToCache(0);
                                
                                Interface::Display* newDisplay = new Interface::Display(app->m_Loader->m_Profiler, app->m_Logger, app->m_Loader, 1);

                                newDisplay->Initialize(*app->m_OcioModule);
                                
                                app->m_Displays[++app->m_DisplayCount] = std::make_pair(true, newDisplay);
                                app->m_ActiveDisplayID = 1;
                            }
                            
                        }
                    }
                }
            }

            ImGui::End();
        }
    }
}