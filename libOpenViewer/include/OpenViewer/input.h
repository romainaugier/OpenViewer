// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#include "OpenViewer/openviewer.h"

#include "tsl/robin_map.h"

LOV_NAMESPACE_BEGIN

// ** Image input functions **

// For now, we declare them here and define them in input.cpp src file. Later,
// we'll make a proper per input func dll with an interface for custom dll loading

void exr_input_func(void* __restrict buffer, const std::string& path) noexcept;
void png_input_func(void* __restrict buffer, const std::string& path) noexcept;
void jpg_input_func(void* __restrict buffer, const std::string& path) noexcept;

// The any prefix means that this function will be returned when the given extension cannot be found in
// the registered functions
void any_input_func(void* __restrict buffer, const std::string& path) noexcept;

// To load images inside memory, we use a system of function pointers registered inside a map 
// with the key being the image format extension, and the value being the function pointer
// That way, users can easily register their own functions for any image format they want to load 
//
//
// An image input function needs to take a void ptr argument, which will be given by the image cache to store
// the image data, and a const std::string& argument, which will be the path to the image file (and any other optional data),
// for example for layers in exr files, we format the filepath like this :
// /path/to/image.exr#layer_name#r,g,b,a

using image_input_func = void(*)(void*, const std::string&);

class LOV_DLL InputFuncs
{
public:
    // Returns an instance of the settings
    static InputFuncs& get_instance() noexcept { static InputFuncs s; return s; }

    InputFuncs(const InputFuncs&) = delete;
    InputFuncs& operator=(const InputFuncs&) = delete;

    // Register a new image input function given an image extension. If the extension is already associated 
    // with an input function, it will be overriden
    LOV_FORCEINLINE void register_image_input_func(const std::string& extension, image_input_func func) noexcept
    {
        spdlog::debug("[INPUT] : Registered image input function for extension {}", extension);
        
        this->m_funcs[extension] = func;
    }

    // Returns an image input function given an extension
    LOV_FORCEINLINE image_input_func get_image_input_func(const std::string& extension) noexcept
    {
        if(this->m_funcs.find(extension) != this->m_funcs.end()) 
        {
            return this->m_funcs[extension];
        }
        else
        {
            spdlog::error("[INPUT] : Can't find image input function for extension {}", extension);
            return nullptr;
        }
    }

    // [] operator
    auto operator [] (const std::string& key) noexcept { return this->m_funcs[key]; }

private:
    InputFuncs() 
    { 
        this->register_image_input_func("exr", exr_input_func);
        this->register_image_input_func("png", png_input_func);
        this->register_image_input_func("jpg", jpg_input_func);
        this->register_image_input_func("any", any_input_func);
    }  
    ~InputFuncs() {}

    tsl::robin_map<std::string, image_input_func> m_funcs;
};



LOV_NAMESPACE_END