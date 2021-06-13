// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.


#include "ocio.h"

void Ocio::Initialize()
{
    try
    {
        config = OCIO::GetCurrentConfig();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';

        // TODO implement config from file
    }
    
}

void Ocio::GetOcioActiveViews()
{
    
}