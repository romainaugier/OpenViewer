// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#include "OpenViewer/media.h"

#include "tsl/ordered_map.h"

LOV_NAMESPACE_BEGIN

using media_map = tsl::ordered_map<std::string, Media*>;

class LOV_API MediaPool
{
public:
#if defined(LOV_MEDIAPOOL_SINGLETON)
    static MediaPool& get_instance() noexcept { static MediaPool mp; return mp; }

    MediaPool(const MediaPool&) = delete;
    MediaPool& operator=(const MediaPool&) = delete;
#else
    MediaPool();
    ~MediaPool();
#endif // defined(LOV_MEDIAPOOL_SINGLETON)

    // This method does the heavy lifting
    void add_media(std::string media_path) noexcept;

    void remove_media(const std::string& media_path) noexcept;

    LOVU_FORCEINLINE media_map get_medias() const noexcept { return this->m_medias; }

    LOVU_FORCEINLINE Media* get_media(const std::string& media_path) noexcept { return this->m_medias.find(media_path) != this->m_medias.end() ? this->m_medias[media_path] : nullptr; }
    
    LOVU_FORCEINLINE Media* get_media(const uint32_t media_index) noexcept { return this->m_medias.values_container()[media_index].second; }

    void debug_media() const noexcept;

private:
#if defined(LOV_MEDIAPOOL_SINGLETON)
    MediaPool();
    ~MediaPool();
#endif // defined(LOV_MEDIAPOOL_SINGLETON)
    media_map m_medias;
};

#if defined(LOV_MEDIAPOOL_SINGLETON)
#define mediapool MediaPool::get_instance()
#else
#define mediapool
#endif // defined(LOV_MEDIAPOOL_SINGLETON)

LOV_NAMESPACE_END
