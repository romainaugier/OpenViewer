// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#include "OpenViewer/openviewer.h"
#include "OpenViewer/media.h"

LOV_NAMESPACE_BEGIN

class LOV_API Ocio
{
    std::vector<std::string> m_views;
    std::vector<std::string> m_displays;
    std::vector<std::string> m_role_names;
    std::vector<std::string> m_role_colorspaces;
    std::vector<std::string> m_looks;

    std::string m_config_path;

    OCIO::ConstConfigRcPtr m_config = nullptr;

    OCIO::ConstCPUProcessorRcPtr m_cpu_processor = nullptr;
    OCIO::ConstGPUProcessorRcPtr m_gpu_processor = nullptr;
    OCIO::ConstProcessorRcPtr m_processor = nullptr;

    uint16_t m_current_view = 0;
    uint16_t m_current_display = 0;
    uint16_t m_current_role = 0;
    uint16_t m_current_look = 0;

    bool m_use_gpu : 1;

public:
    Ocio();

    ~Ocio();

    //
    void set_config(const std::string& config_path) noexcept;

    //
    void update_processor() noexcept;

    // 
    void update_cpu_processor(const OCIO::BitDepth bit_depth) noexcept;
    
    //
    void process_cpu(void* pixels, 
                     const uint16_t width, 
                     const uint16_t height, 
                     const uint8_t nchannels, 
                     const uint8_t type) noexcept;

    //
    void process_gpu(void* __restrict pixels, const bool has_alpha) noexcept;

    //
    void update_views() noexcept;

    //
    LOV_FORCEINLINE void set_current_view(const uint16_t index) noexcept { this->m_current_view = index; }
    LOV_FORCEINLINE void set_current_display(const uint16_t index) noexcept { this->m_current_display = index; this->update_views(); }
    LOV_FORCEINLINE void set_current_role(const uint16_t index) noexcept { this->m_current_role = index; }
    LOV_FORCEINLINE void set_current_look(const uint16_t index) noexcept { this->m_current_look = index; }

    //
    void set_view_from_str(const char* view) noexcept;
    void set_display_from_str(const char* display) noexcept;
    void set_role_from_str(const char* role) noexcept;
    void set_look_from_str(const char* look) noexcept;

    //
    LOV_FORCEINLINE std::string get_current_view() const noexcept { return this->m_views[this->m_current_view]; }
    LOV_FORCEINLINE std::string get_current_display() const noexcept { return this->m_displays[this->m_current_display]; }
    LOV_FORCEINLINE std::string get_current_role_name() const noexcept { return this->m_role_names[this->m_current_role]; }
    LOV_FORCEINLINE std::string get_current_role_colorspace() const noexcept { return this->m_role_colorspaces[this->m_current_role]; }
    LOV_FORCEINLINE std::string get_current_look() const noexcept { return this->m_looks[this->m_current_look]; }

    //
    LOV_FORCEINLINE std::vector<std::string> get_views() const noexcept { return this->m_views; }
    LOV_FORCEINLINE std::vector<std::string> get_displays() const noexcept { return this->m_displays; }
    LOV_FORCEINLINE std::vector<std::string> get_role_names() const noexcept { return this->m_role_names; }
    LOV_FORCEINLINE std::vector<std::string> get_role_colorspaces() const noexcept { return this->m_role_colorspaces; }
    LOV_FORCEINLINE std::vector<std::string> get_looks() const noexcept { return this->m_looks; }

    //
    LOV_FORCEINLINE void set_use_cpu() noexcept { this->m_use_gpu = false; }
    LOV_FORCEINLINE void set_use_gpu() noexcept { this->m_use_gpu = true; }

    //
    void debug() const noexcept;
};

LOV_NAMESPACE_END