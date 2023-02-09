// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/ocio.h"
#include "OpenViewerUtils/filesystem.h"


LOV_NAMESPACE_BEGIN

Ocio::Ocio()
{
    this->m_use_gpu = false;

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
        spdlog::debug("[OCIO] : Set config from \"{}\"", this->m_config_path);
    }
    catch(const std::exception& e)
    {
        spdlog::error("[OCIO] : Catched exception during config parsing : {}", e.what());
        std::exit(EXIT_FAILURE);
    }

    // Gettings the roles
    this->m_role_names.clear();
    this->m_role_colorspaces.clear();

    const uint16_t num_roles = this->m_config->getNumRoles();
    this->m_role_names.reserve(num_roles);
    this->m_role_colorspaces.reserve(num_roles);

    for(uint16_t i = 0; i < num_roles; i++)
    {
        this->m_role_names.emplace_back(this->m_config->getRoleName(i));
        this->m_role_colorspaces.emplace_back(this->m_config->getRoleColorSpace(i));
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
    spdlog::debug("[OCIO] : Updating processors");
    
    const char* _role = this->m_config->getRoleColorSpace(this->m_current_role);
    const char* _display = this->m_config->getDisplay(this->m_current_display);
    const char* _view = this->m_config->getView(_display, this->m_current_view);

    spdlog::debug("[OCIO] : Current role : {}", _role);
    spdlog::debug("[OCIO] : Current display : {}", _display);
    spdlog::debug("[OCIO] : Current view : {}", _view);

    const OCIO::DisplayViewTransformRcPtr transform = OCIO::DisplayViewTransform::Create();
    transform->setSrc(_role);
    transform->setDisplay(_display);
    transform->setView(_view);
    transform->setDirection(OCIO::TRANSFORM_DIR_FORWARD);

    const OCIO::LegacyViewingPipelineRcPtr vp = OCIO::LegacyViewingPipeline::Create();
    vp->setDisplayViewTransform(transform);

    if(this->m_current_look != 0)
    {
        vp->setLooksOverrideEnabled(true);
        vp->setLooksOverride(this->get_current_look().c_str());
    }
    else
    {
        vp->setLooksOverrideEnabled(false);
    }

    this->m_processor = vp->getProcessor(this->m_config, this->m_config->getCurrentContext());

    spdlog::debug("[OCIO] : Updated Processor");
}

void Ocio::update_cpu_processor(const OCIO::BitDepth bit_depth) noexcept
{
    this->m_cpu_processor = this->m_processor->getOptimizedCPUProcessor(bit_depth, bit_depth, OCIO::OPTIMIZATION_ALL);

    spdlog::debug("[OCIO] : Updated CPU Processor");
}

void Ocio::process_cpu(void* pixels, 
                       const uint16_t width, 
                       const uint16_t height, 
                       const uint8_t nchannels, 
                       const uint8_t type) noexcept
{
    try
    {
        const OCIO::BitDepth bit_depth = TYPE_TO_OCIO_BIT_DEPTH(type);

        const OCIO::ChannelOrdering chan_ord = nchannels > 3 ? OCIO::CHANNEL_ORDERING_RGBA : OCIO::CHANNEL_ORDERING_RGB;

        OCIO::PackedImageDesc img(pixels, width, height, chan_ord, bit_depth, OCIO::AutoStride, OCIO::AutoStride, OCIO::AutoStride);

        if(this->m_cpu_processor != nullptr)
        {
            this->m_cpu_processor->apply(img);
        }
    }
    catch(const std::exception& e)
    {
        spdlog::error("[OCIO] : Exception catched during cpu processing : {}", e.what());
    }
}

void Ocio::process_gpu(void* __restrict pixels, const bool has_alpha) noexcept
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

void Ocio::set_view_from_str(const char* view) noexcept
{
    const auto it = std::find(this->m_views.begin(), this->m_views.end(), view);

    if(it == this->m_views.end())
    {
        spdlog::warn("[OCIO] : Can't find view \"{}\". Keeping previous settings", view);
        return;
    }

    this->m_current_view = it - this->m_views.begin();
}

void Ocio::set_display_from_str(const char* display) noexcept
{
    const auto it = std::find(this->m_displays.begin(), this->m_displays.end(), display);

    if(it == this->m_displays.end())
    {
        spdlog::warn("[OCIO] : Can't find display \"{}\". Keeping previous settings", display);
        return;
    }

    this->m_current_display = it - this->m_displays.begin();

    this->update_views();
}

void Ocio::set_role_from_str(const char* role) noexcept
{
    auto it = std::find(this->m_role_names.begin(), this->m_role_names.end(), role);

    if(it == this->m_role_names.end())
    {
        it = std::find(this->m_role_colorspaces.begin(), this->m_role_colorspaces.end(), role);

        if(it == this->m_role_colorspaces.end())
        {
            spdlog::warn("[OCIO] : Can't find role \"{}\". Keeping previous settings \"{}\"", role, this->get_current_role_name());
            return;
        }
        
        this->m_current_role = it - this->m_role_colorspaces.begin();
    }
    else
    {
        this->m_current_role = it - this->m_role_names.begin();
    }
}

void Ocio::set_look_from_str(const char* look) noexcept
{
    const auto it = std::find(this->m_looks.begin(), this->m_looks.end(), look);

    if(it == this->m_looks.end())
    {
        spdlog::warn("[OCIO] : Can't find look \"{}\". Keeping previous settings", look);
        return;
    }

    this->m_current_look = it - this->m_looks.begin();

}


void Ocio::debug() const noexcept
{
    
    spdlog::debug("************************");

    spdlog::debug("[OCIO] : Debugging current OCIO State");

    spdlog::debug("Current OCIO configuration path : {}", this->m_config_path);

    spdlog::debug("Roles : ");
    
    for(size_t i = 0; i < this->m_role_names.size(); i++)
    {
        spdlog::debug("  - {} : {}", this->m_role_names[i], this->m_role_colorspaces[i]);
    }

    spdlog::debug("Displays : ");
    for(const auto& display : this->m_displays) spdlog::debug("  - {}", display);

    spdlog::debug("Views : ");
    for(const auto& view : this->m_views) spdlog::debug("  - {}", view);

    spdlog::debug("Looks : ");
    for(const auto& look : this->m_looks) spdlog::debug("  - {}", look);

    spdlog::debug("Current role : {} : {}", 
                  this->m_role_names[this->m_current_role], 
                  this->m_role_colorspaces[this->m_current_role]);

    spdlog::debug("Current display : {}", this->m_displays[this->m_current_display]);
    spdlog::debug("Current view : {}", this->m_views[this->m_current_view]);
    spdlog::debug("Current look : {}", this->m_looks[this->m_current_look]);

    spdlog::debug("************************");
}

LOV_NAMESPACE_END