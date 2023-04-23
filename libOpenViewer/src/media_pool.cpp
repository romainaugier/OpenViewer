// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/media_pool.h"
#include "OpenViewer/settings.h"
#include "OpenViewerUtils/filesystem.h"

LOV_NAMESPACE_BEGIN

MediaPool::MediaPool() 
{
    spdlog::debug("[MEDIA POOL] : Initialized Media Pool");
}

MediaPool::~MediaPool() 
{
    spdlog::debug("[MEDIA POOL] : Released Media Pool");
}

void MediaPool::add_media(std::string media_path) noexcept
{
    if(lovu::fs::is_image(media_path))
    {
        const bool autodetect_sequence = settings.get<bool>("autodetect_sequences");

        bool found_sequence = false;

        if(autodetect_sequence)
        {
            const std::string sequence_path = lovu::fs::get_file_sequence_from_file(media_path); 

            if(sequence_path != "") 
            {
                found_sequence = true;
                media_path = std::move(sequence_path);
            }
        }

        if(found_sequence)
        {
            this->m_medias[media_path] = new ImageSequence(media_path);
            spdlog::debug("[MEDIA POOL] : Added a new image sequence media : \"{}\"", media_path);
        }
        else
        {
            this->m_medias[media_path] = new Image(media_path);
            spdlog::debug("[MEDIA POOL] : Added a new image media : \"{}\"", media_path);
        }
    }
    else if(lovu::fs::is_video(media_path))
    {
        spdlog::error("Videos are not supported for now");
    }
    else
    {
        spdlog::error("Media \"{}\" not supported", media_path);
    }
}

void MediaPool::remove_media(const std::string& media_path) noexcept
{
    Media* media = this->m_medias[media_path];
    
    this->m_medias.erase(media_path);
    
    delete media;

    spdlog::debug("[MEDIA POOL] : Removed media \"{}\"", media_path);
}

void MediaPool::debug_media() const noexcept
{
    spdlog::debug("************************");
    spdlog::debug("[MEDIA POOL] : Debugging medias that are currently in the media pool");

    for(auto& [media_path, media] : this->m_medias)
    {
        spdlog::debug("  Media : {}", media_path);
    }

    spdlog::debug("************************");
}

LOV_NAMESPACE_END
