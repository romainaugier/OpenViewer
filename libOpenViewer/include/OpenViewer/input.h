// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#include "openviewer.h"

#include "tsl/robin_map.h"

LOV_NAMESPACE_BEGIN

// ** Image input functions **

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

// Map holding image format extensions and their associated input functions
tsl::robin_map<std::string, image_input_func> image_input_funcs;

// Register a new image input function given an image extension. If the extension is already associated 
// with an input function, it will be overriden
LOV_FORCEINLINE void register_image_input_func(const std::string& extension, image_input_func func) noexcept
{
    spdlog::debug("[INPUT] : Registered image input function for extension {}", extension);
    
    image_input_funcs[extension] = func;
}

// Returns an image input function given an extension
LOV_FORCEINLINE image_input_func get_image_input_func(const std::string& extension) noexcept
{
    if(image_input_funcs.find(extension) != image_input_funcs.end()) 
    {
        return image_input_funcs[extension];
    }
    else
    {
        spdlog::error("[INPUT] : Can't find image input function for extension {}", extension);
        return nullptr;
    }
}

LOV_NAMESPACE_END