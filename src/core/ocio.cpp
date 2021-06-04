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