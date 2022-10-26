// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/openviewer.h"

#include "nlohmann/json.hpp"

LOV_NAMESPACE_BEGIN

// To store settings, we use a simple singleton to ease access to settings at various 
// places of the program

using json = nlohmann::json;

#define SETTINGS_FILE_PATH "OpenViewer/settings.json"

class LOV_DLL Settings
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
    
    // Getter by return type
    template <class T>
    struct item_return { using type = T; };
    
    template <class T>
    typename item_return<T>::type get(const std::string& key) const noexcept { return this->m_data[key].get<T>(); }

    // template<>
    // struct item_return<int> { using type = int; };
    // template<>
    // int get<int>(const std::string& key) const noexcept { return this->m_data[key].get<int>(); }
    
    // template<>
    // struct item_return<unsigned int> { using type = unsigned int; };
    // template<>
    // unsigned int get<unsigned int>(const std::string& key) const noexcept { return this->m_data[key].get<unsigned int>(); }

    // template<>
    // struct item_return<float> { using type = float; };
    // template<>
    // float get<float>(const std::string& key) const noexcept { return this->m_data[key].get<float>(); }

    // template<>
    // struct item_return<bool> { using type = bool; };
    // template<>
    // bool get<bool>(const std::string& key) const noexcept { return this->m_data[key].get<bool>(); }

    // template<>
    // struct item_return<std::string> { using type = std::string; };
    // template<>
    // std::string get<std::string>(const std::string& key) const noexcept { return this->m_data[key].get<std::string>(); }

    // [] operator
    auto operator [] (const std::string& key) const noexcept { return this->m_data[key]; }
    auto operator [] (const std::string& key) noexcept { return this->m_data[key]; }

private:
    Settings() {}
    ~Settings() {}

    json m_data;
};

// A little macro to have cleaner and more understandable code
#define settings Settings::get_instance()

LOV_NAMESPACE_END