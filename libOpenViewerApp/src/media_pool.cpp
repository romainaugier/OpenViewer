// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewerApp/media_pool.h"

#define LOV_MEDIAPOOL_SINGLETON
#include "OpenViewer/media_pool.h"

#include "imgui.h"

LOVA_NAMESPACE_BEGIN

MediaPoolWidget::MediaPoolWidget()
{

}

MediaPoolWidget::~MediaPoolWidget()
{

}

void MediaPoolWidget::draw() noexcept
{
    if(this->m_show)
    {
        ImGui::Begin("Media Pool", &this->m_show);
        {
            static bool selected = false;   

            for(const auto& [media_path, media] : lov::mediapool.get_medias())
            {
                ImGui::Selectable(media_path.c_str(), selected);
            }
        }
        ImGui::End();
    }
}

LOVA_NAMESPACE_END