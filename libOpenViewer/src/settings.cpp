// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/settings.h"
#include "OpenViewerUtils/filesystem.h"
#include "fmt/format.h"
#include <fstream>

LOV_NAMESPACE_BEGIN

void Settings::save() const noexcept
{
    const std::string settings_file_path = fmt::format("{}/{}", lovu::fs::get_documents_folder_path(), SETTINGS_FILE_PATH);
    const std::string open_viewer_pref_dir = std::filesystem::path(settings_file_path).parent_path().string();

    if(!lovu::fs::exists(open_viewer_pref_dir))
    {
        spdlog::debug("Can't find OpenViewer preferences directory, creating it");
        std::filesystem::create_directory(open_viewer_pref_dir);
    }

    spdlog::debug("Saving settings");

    std::ofstream file(settings_file_path);

    file << std::setw(4) << this->m_data << std::endl;
}


void Settings::load() noexcept
{
    const std::string settings_file_path = fmt::format("{}/{}", lovu::fs::get_documents_folder_path(), SETTINGS_FILE_PATH);

    if(lovu::fs::exists(settings_file_path))
    {
        spdlog::debug("Found settings file, loading settings");
        
        std::ifstream file(settings_file_path);

        file >> this->m_data;
    }
    else
    {
        spdlog::debug("Cannot find settings file, using default settings");

        this->m_data = {
            { "autodetect_sequences", true },
            { "openexr_thread_count", 8 },
            { "cache_mode", 2 },
            { "cache_max_ram_usage", 50 },
            { "cache_max_size", 0 },
            { "log_level",  "info" }
        };
    }

    this->m_loaded = true;
}

LOV_NAMESPACE_END