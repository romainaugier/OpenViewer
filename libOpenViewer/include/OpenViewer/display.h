// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/openviewer.h"
#include "GL/glew.h"

LOV_NAMESPACE_BEGIN

enum DisplayShaderType
{
    DisplayShaderType_OpenGL = LOVU_BIT(0),
    DisplayShaderType_OpenCL = LOVU_BIT(1),
    DisplayShaderType_C      = LOVU_BIT(2),
    DisplayShaderType_Cuda   = LOVU_BIT(3),
    DisplayShaderType_Metal  = LOVU_BIT(4),
    DisplayShaderType_Vulcan = LOVU_BIT(5),
    DisplayShaderType_OSL    = LOVU_BIT(6)
};

using display_shader_func = void(*)(void*, void*);

struct DisplayShader
{
    display_shader_func m_func;
    void* m_parameters;
    uint8_t m_shader_type; 
};

enum DisplayFlag
{
    DisplayFlag_EnableGPU = LOVU_BIT(0)
};

enum DisplayDataType
{
    DisplayDataType_RBGHalfFloat  = 0,
    DisplayDataType_RBGAHalfFloat = 1,
    DisplayDataType_RGBFloat      = 2,
    DisplayDataType_RGBAFloat     = 3,
    DisplayDataType_RGBUByte      = 4,
    DisplayDataType_RGBAUByte     = 5,
    DisplayDataType_RGBUShort     = 6,
    DisplayDataType_RGBAUShort    = 7,
    DisplayDataType_RGBUInteger   = 8,
    DisplayDataType_RGBAUInteger  = 9
};

class LOV_API Display
{
    void set_data(void* data_ptr, const uint32_t data_width, const uint32_t data_height, const DisplayDataType data_type);

    LOV_FORCEINLINE bool has_flag(const DisplayFlag flag) const noexcept { return LOVU_HAS_FLAG(this->m_flags, flag); }

private:
    void* m_data = nullptr;

    GLuint m_gl_texture = 0xffffffff;

    uint32_t m_width = 0xffffffff;
    uint32_t m_height = 0xffffffff;
    
    uint32_t m_flags = 0;

    uint8_t m_data_type = 0;
};

LOV_NAMESPACE_END