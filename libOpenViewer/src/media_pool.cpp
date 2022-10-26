// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/media_pool.h"

LOV_NAMESPACE_BEGIN

MediaPool::MediaPool() 
{
    spdlog::debug("[MEDIA POOL] : Initialized Media Pool");
}

MediaPool::~MediaPool() 
{
    spdlog::debug("[MEDIA POOL] : Released Media Pool");
}

void MediaPool::add_media(const std::string_view& media_path) noexcept
{

}

void MediaPool::remove_media(const std::string_view& media_path) noexcept
{

}

void MediaPool::debug_media() const noexcept
{

}

LOV_NAMESPACE_END