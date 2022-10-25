// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewerUtils/gl.h"

#include <fstream>
#include <sstream>

LOVU_NAMESPACE_BEGIN

GL_NAMESPACE_BEGIN

void Shader::load_and_compile(const char* vertexShaderPath, const char* fragmentShaderPath) noexcept
{
    // Read shader files
    std::ifstream vertexShaderFile, fragmentShaderFile;
    std::string vertexShaderCodeStr, fragmentShaderCodeStr;

    vertexShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    fragmentShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        vertexShaderFile.open(vertexShaderPath);
        fragmentShaderFile.open(fragmentShaderPath);

        std::stringstream vertexShaderStream, fragmentShaderStream;
        
        vertexShaderStream << vertexShaderFile.rdbuf();
        fragmentShaderStream << fragmentShaderFile.rdbuf();

        vertexShaderFile.close();
        fragmentShaderFile.close();

        vertexShaderCodeStr = vertexShaderStream.str();
        fragmentShaderCodeStr = fragmentShaderStream.str();
    }
    catch(std::ifstream::failure e)
    {
        spdlog::error("[OPENGL] : Shader loading stringstream exception : {}", e.what());
    }

    const char* vertexShaderCode = vertexShaderCodeStr.c_str();
    const char* fragmentShaderCode = fragmentShaderCodeStr.c_str();

    // Compile shaders
    uint32_t vertexShader, fragmentShader;
    int32_t success;
    char infoLog[512];

    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderCode, nullptr);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        spdlog::error("[OPENGL] : Vertex shader compilation failed : {}", infoLog);
    }

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderCode, nullptr);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        spdlog::error("[OPENGL] : Fragment shader compilation failed : {}", infoLog);
    }

    // Create program
    this->m_ID = glCreateProgram();
    glAttachShader(this->m_ID, vertexShader);
    glAttachShader(this->m_ID, fragmentShader);
    glLinkProgram(this->m_ID);
    glGetProgramiv(this->m_ID, GL_LINK_STATUS, &success);

    if (!success)
    {
        glGetProgramInfoLog(this->m_ID, 512, nullptr, infoLog);
        spdlog::error("[OPENGL] : Program linking failed : {}", infoLog);
    }

    // Delete shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Shader::use() noexcept
{
    glUseProgram(this->m_ID);
}

void Shader::set_int(const char* name, int value) const noexcept
{
    glUniform1i(glGetUniformLocation(this->m_ID, name), value);
}

void Shader::set_float(const char* name, float value) const noexcept
{
    glUniform1f(glGetUniformLocation(this->m_ID, name), value);
}
 
GL_NAMESPACE_END

LOVU_NAMESPACE_END