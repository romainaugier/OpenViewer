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

// The shader function should accept two arguments : 
// - a void pointer to the data, depending on the shader type it will be different :
//    - OpenGL : Pointer to the texture
//    - OpenCL : Pointer to the OpenGL texture  
//    - C      : Pointer to the data
//    - Others are not implemented yet

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
    DisplayDataType_RGBHalfFloat  = 0,
    DisplayDataType_RGBAHalfFloat = 1,
    DisplayDataType_RGBFloat      = 2,
    DisplayDataType_RGBAFloat     = 3,
    DisplayDataType_RGBUByte      = 4,
    DisplayDataType_RGBAUByte     = 5,
    DisplayDataType_RGBUShort     = 6,
    DisplayDataType_RGBAUShort    = 7,
    DisplayDataType_RGBUInteger   = 8,
    DisplayDataType_RGBAUInteger  = 9
};

#define DISPLAY_DATA_TYPE_TO_GL_INTERNAL_FMT(data_type)              \
        data_type == DisplayDataType_RGBHalfFloat ? GL_RGB16F :      \
        data_type == DisplayDataType_RGBAHalfFloat ? GL_RGBA16F :    \
        data_type == DisplayDataType_RGBFloat ? GL_RGB32F :          \
        data_type == DisplayDataType_RGBAFloat ? GL_RGBA32F :        \
        data_type == DisplayDataType_RGBUByte ? GL_RGB8UI :          \
        data_type == DisplayDataType_RGBAUByte ? GL_RGBA8UI :        \
        data_type == DisplayDataType_RGBUShort ? GL_RGB16UI :        \
        data_type == DisplayDataType_RGBAUShort ? GL_RGBA16UI :      \
        data_type == DisplayDataType_RGBUInteger ? GL_RGB32UI :      \
        data_type == DisplayDataType_RGBAUInteger ? GL_RGBA32UI : 10 \

#define DISPLAY_DATA_TYPE_TO_GL_FMT(data_type) data_type % 2 == 0 ? GL_RGB : GL_RGBA

#define DISPLAY_DATA_TYPE_TO_GL_TYPE(data_type)                             \
        data_type >= 0 && data_type < 2 ? GL_HALF_FLOAT :                   \
        data_type >= 2 && data_type < 4 ? GL_FLOAT :                        \
        data_type >= 4 && data_type < 6 ? GL_UNSIGNED_BYTE :                \
        data_type >= 6 && data_type < 8 ? GL_UNSIGNED_SHORT :               \
        data_type >= 8 && data_type < 10 ? GL_UNSIGNED_INT : GL_HALF_FLOAT  \

class LOV_API Display
{
public:
    Display();

    ~Display();

    void add_shader(const DisplayShader& shader) noexcept { this->m_shaders.emplace_back(std::move(shader)); }

    void run_shaders() const noexcept;

    void set_data(void* data_ptr,
                  const uint32_t data_width,
                  const uint32_t data_height,
                  const uint8_t data_type) noexcept;

    LOV_FORCEINLINE bool has_flag(const DisplayFlag flag) const noexcept { return LOVU_HAS_FLAG(this->m_flags, flag); }

private:
    std::vector<DisplayShader> m_shaders;

    void* m_data = nullptr;

    GLuint m_gl_texture;

    uint32_t m_width = 0;
    uint32_t m_height = 0;
    
    uint32_t m_flags = 0;

    uint8_t m_data_type = 0;
};

LOV_NAMESPACE_END