// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/media.h"

#include "tsl/robin_map.h"

LOV_NAMESPACE_BEGIN

using media_map = tsl::robin_map<std::string, Media*>;

class LOV_DLL MediaPool
{
public:
    MediaPool();
    
    ~MediaPool();

    // This method does the heavy lifting
    void add_media(std::string& media_path) noexcept;

    void remove_media(const std::string& media_path) noexcept;

    LOVU_FORCEINLINE media_map get_medias() const noexcept { return this->m_medias; }

    void debug_media() const noexcept;

private:
    media_map m_medias;
};

LOV_NAMESPACE_END