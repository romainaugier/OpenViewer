// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#include "OpenViewer/openviewer.h"

#include "OpenColorIO/OpenColorIO.h"

namespace OCIO = OCIO_NAMESPACE;

LOV_NAMESPACE_BEGIN

class Ocio
{
    std::vector<std::string> m_views;
    std::vector<std::string> m_displays;
    std::vector<std::string> m_roles;
    std::vector<std::string> m_looks;

    std::string m_config_path;

    OCIO::ConstConfigRcPtr m_config = nullptr;

    OCIO::ConstCPUProcessorRcPtr m_cpu_processor = nullptr;
    OCIO::ConstGPUProcessorRcPtr m_gpu_processor = nullptr;

    uint16_t m_current_view = 0;
    uint16_t m_current_display = 0;
    uint16_t m_current_role = 0;
    uint16_t m_current_look = 0;

public:
    Ocio();

    ~Ocio();

    //
    void set_config(const std::string& config_path) noexcept;

    //
    void update_processor() noexcept;
    
    //
    void process_cpu(const uint16_t img_width, const uint16_t img_height) noexcept;

    //
    void process_gpu(const uint16_t img_width, const uint16_t img_height) noexcept;

    //
    void update_views() noexcept;

    //
    LOV_FORCEINLINE void set_current_view(const uint16_t index) noexcept { this->m_current_view = index; }
    LOV_FORCEINLINE void set_current_display(const uint16_t index) noexcept { this->m_current_display = index; }
    LOV_FORCEINLINE void set_current_role(const uint16_t index) noexcept { this->m_current_role = index; }
    LOV_FORCEINLINE void set_current_look(const uint16_t index) noexcept { this->m_current_look = index; }

    //
    LOV_FORCEINLINE std::string get_current_view() const noexcept { return this->m_views[this->m_current_view]; }
    LOV_FORCEINLINE std::string get_current_display() const noexcept { return this->m_displays[this->m_current_display]; }
    LOV_FORCEINLINE std::string get_current_role() const noexcept { return this->m_roles[this->m_current_role]; }
    LOV_FORCEINLINE std::string get_current_look() const noexcept { return this->m_looks[this->m_current_look]; }

    //
    LOV_FORCEINLINE std::vector<std::string> get_views() const noexcept { return this->m_views; }
    LOV_FORCEINLINE std::vector<std::string> get_displays() const noexcept { return this->m_displays; }
    LOV_FORCEINLINE std::vector<std::string> get_roles() const noexcept { return this->m_roles; }
    LOV_FORCEINLINE std::vector<std::string> get_looks() const noexcept { return this->m_looks; }
};

LOV_NAMESPACE_END