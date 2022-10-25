// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include "GL/glew.h"

#include "string_utils.h"
#include "logger.h"

inline void CheckOpenGLError(const char* stmt, const char* fname, int line)
{
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        printf("OpenGL error %08x, at %s:%i - for %s\n", err, fname, line, stmt);
    }
}

#ifdef _DEBUG
#define GL_CHECK(stmt) do { \
            stmt; \
            CheckOpenGLError(#stmt, __FILE__, __LINE__); \
        } while (0)
#else
#define GL_CHECK(stmt) stmt
#endif

namespace Utils
{
    namespace GL
    {
        struct Shader
        {
            uint32_t m_ID;

            void LoadAndCompile(const char* vertexShaderPath, const char* fragmentShaderPath) noexcept;

            void Use() noexcept;

            void SetInt(const char* name, int value) const noexcept;

            void SetFloat(const char* name, float value) const noexcept;
        };
    } // End namespace GL
} // End namespace Utils