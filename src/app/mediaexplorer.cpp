// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "mediaexplorer.h"

namespace Interface
{
    MediaExplorer::MediaExplorer(Core::Loader* loader)
    {
        this->m_Loader = loader;
    }

    void MediaExplorer::Draw(bool& showWindow) noexcept
    {
        if (showWindow)
        {
            ImGui::Begin("Media Explorer", &showWindow);
            {
                static bool selected = false;

                for (uint32_t i = 0; i < this->m_Loader->m_Medias.size(); i++)
                {
                    const std::string mediaPath = this->m_Loader->m_Medias[i].m_Images[0].m_Path;

                    if (ImGui::Selectable(mediaPath.c_str(), selected))
                    {
                        printf("%s\n", mediaPath.c_str());
                    }
                }
            }
            ImGui::End();
        }
    }
}