// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/ocio.h"
#include "OpenViewerUtils/filesystem.h"

LOV_NAMESPACE_BEGIN

Ocio::Ocio()
{
    spdlog::debug("[OCIO] : Initializing OCIO");

    const char* ocio_env_var_path = std::getenv("OCIO");

    if(ocio_env_var_path != nullptr)
    {
        this->set_config(ocio_env_var_path);
        spdlog::debug("[OCIO] : Loaded config from environment variable \"OCIO\"");
    }
    else
    {
        spdlog::info("[OCIO] : Can't find OCIO environment variable, falling back to default shipped config");
        this->set_config(lovu::fs::expand_from_executable_dir("/configs/default/default.ocio"));
        spdlog::debug("[OCIO] : Loaded config from default shipped configuration");
    }
}

Ocio::~Ocio()
{
    spdlog::debug("[OCIO] : Releasing OCIO");
}

void Ocio::set_config(const std::string& config_path) noexcept
{
    this->m_current_view = 0;
    this->m_current_display = 0;
    this->m_current_role = 0;
    this->m_current_look = 0;

    this->m_config_path = std::move(config_path);

    try
    {
        this->m_config = OCIO::Config::CreateFromFile(this->m_config_path.c_str());
        spdlog::debug("[OCIO] : Set config from \"{}\"", this->m_config);
    }
    catch(const std::exception& e)
    {
        spdlog::error("[OCIO] : Catched exception during config parsing : {}", e.what());
        std::exit(EXIT_FAILURE);
    }

    // Gettings the roles
    this->m_roles.clear();

    const uint16_t num_roles = this->m_config->getNumRoles();
    this->m_roles.reserve(num_roles);

    for(uint16_t i = 0; i < num_roles; i++)
    {
        this->m_roles.emplace_back(fmt::format("{} : {}",
                                               this->m_config->getRoleColorSpace(i),
                                               this->m_config->getRoleName(i)));
    }

    // Getting the displays
    this->m_displays.clear();
    const char* active_displays = this->m_config->getActiveDisplays();
    lovu::str::split(this->m_displays, active_displays, ',');

    for(auto& display : this->m_displays) lovu::str::strip(display);

    // Gettings the views 
    this->update_views();

    // Gettings the looks
    this->m_looks.clear();

    this->m_looks.emplace_back("None");

    for(uint16_t i = 0; i < this->m_config->getNumLooks(); i++)
    {
        std::string tmp = this->m_config->getLookNameByIndex(i);

        lovu::str::strip(tmp);

        this->m_looks.push_back(tmp);
    }
}

void Ocio::update_processor() noexcept
{

}

void Ocio::process_cpu(const uint16_t img_width, const uint16_t img_height) noexcept
{

}

void Ocio::process_gpu(const uint16_t img_width, const uint16_t img_height) noexcept
{

}

void Ocio::update_views() noexcept
{
    this->m_views.clear();

    uint16_t i = 0;

    while(true)
    {
        std::string tmp = this->m_config->getView(this->get_current_display().c_str(), i);

        if(tmp.empty()) break;

        lovu::str::strip(tmp);

        this->m_views.push_back(tmp);

        ++i;
    }
}

LOV_NAMESPACE_END