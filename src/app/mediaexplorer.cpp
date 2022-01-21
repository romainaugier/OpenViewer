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

                for (auto& [id, media] : this->m_Loader->m_Medias)
                {
                    const std::string mediaPath = media->Path();

                    const bool isSequence = media->GetRange().y > 1;

                    char selectableLabel[8192];
                    
                    if (isSequence) Utils::Str::Format(selectableLabel, "%s (%d images)", mediaPath.c_str(), 
                                                                                          static_cast<uint32_t>(media->GetRange().y));
                    else Utils::Str::Format(selectableLabel, "%s (1 image)", mediaPath.c_str());

                    ImGui::Selectable(selectableLabel, selected);

                    if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
                    {    
                        this->m_Logger->Log(LogLevel_Diagnostic, "[MEDIA] : Media %s [ID : %d] is now active", mediaPath.c_str(), media->ID());

                        this->m_ActiveMediaID = media->ID();
                        this->m_CurrentMediaRange = media->GetRange();

                        // If no display is active, create one
                        if (app->m_DisplayCount == 0)
                        {
                            Interface::Display* newDisplay = new Interface::Display(app->m_Loader->m_Profiler, app->m_Logger, app->m_Loader, 1);

                            newDisplay->Initialize(*app->m_OcioModule, media->ID());
                            newDisplay->NeedFrame();
                            
                            app->m_Displays[++app->m_DisplayCount] = std::make_pair(true, newDisplay);
                        }
                        else
                        {
                            Interface::Display* activeDisplay = app->GetActiveDisplay();
                            activeDisplay->NeedReinit();
                            activeDisplay->NeedFrame();
                            activeDisplay->m_MediaID = media->ID();
                        }

                        app->UpdateCache();
                        this->m_Loader->LoadImageToCache(media->ID(), 0);

                        this->m_CurrentMediaChanged = true;
                    }
                }
            }

            ImGui::End();
        }
    }
}