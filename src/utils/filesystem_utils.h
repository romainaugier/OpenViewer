// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include <filesystem>
#include <regex>

#include "decl.h" 

namespace Utils
{
    OPENVIEWER_FORCEINLINE size_t FileCountInDirectory(const std::string& directoryPath)
    {
        return static_cast<size_t>(std::distance(std::filesystem::directory_iterator(directoryPath), std::filesystem::directory_iterator{}));
    }

    
} // End namespace Utils