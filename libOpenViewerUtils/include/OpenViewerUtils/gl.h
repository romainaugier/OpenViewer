// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#pragma once

#include "GL/glew.h"

#include "string.h"

#define GL_NAMESPACE_BEGIN namespace gl {
#define GL_NAMESPACE_END }

LOVU_NAMESPACE_BEGIN

GL_NAMESPACE_BEGIN

// Utility function and macro to check for opengl errors        
void check_opengl_error(const char* stmt, const char* fname, int line)
{
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        spdlog::error("OpenGL error {0:#x}, at {:s}:{:d} - for {:s}\n", err, fname, line, stmt);
    }
}

#ifdef _DEBUG
#define GL_CHECK(stmt) do { \
            stmt; \
            check_opengl_error(#stmt, __FILE__, __LINE__); \
        } while (0)
#else
#define GL_CHECK(stmt) stmt
#endif

// Utility struct to ease the construction and use of an opengl shader
struct LOVU_API Shader
{
    uint32_t m_ID;

    // Loads and compile the vertex and fragment shader
    void load_and_compile(const char* vertexShaderPath, const char* fragmentShaderPath) noexcept;

    // Uses the shaders
    void use() noexcept;

    // Set an int value in the shader
    void set_int(const char* name, int value) const noexcept;

    //Set a float value in the shader
    void set_float(const char* name, float value) const noexcept;
};

GL_NAMESPACE_END

LOVU_NAMESPACE_END