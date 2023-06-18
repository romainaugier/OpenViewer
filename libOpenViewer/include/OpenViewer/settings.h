// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#include "OpenViewer/openviewer.h"

#include "nlohmann/json.hpp"

LOV_NAMESPACE_BEGIN

// To store settings, we use a simple singleton to ease access to settings at various 
// places of the program

using json = nlohmann::json;

#define SETTINGS_FILE_PATH "OpenViewer/settings.json"

class LOV_API Settings
{
public:
    // Returns an instance of the settings
    static Settings& get_instance() noexcept { static Settings s; return s; }

    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;
  
    // Save the settings to the OpenViewer settings file that is located in the 
    // user home documents folder
    void save() const noexcept;

    // Load the settings from the OpenViewer settings file that is located in the 
    // user home documents folder
    void load() noexcept;
    
    json data() noexcept { return this->m_data; }
    
    // Getter by return type
    template <class T>
    struct item_return { using type = T; };
    
    template <class T>
    typename item_return<T>::type get(const std::string& key) const noexcept { return this->m_data[key].get<T>(); }

    // [] operator
    auto operator [] (const std::string& key) const noexcept { return this->m_data[key]; }
    auto operator [] (const std::string& key) noexcept { return this->m_data[key]; }

private:
    Settings() { this->load(); }  
    ~Settings() { this->save(); }

    json m_data;
    bool m_loaded = false;
};

// A little macro to have cleaner and more understandable code
#define settings Settings::get_instance()

LOV_NAMESPACE_END