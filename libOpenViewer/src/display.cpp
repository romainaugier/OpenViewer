// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewer/display.h"

LOV_NAMESPACE_BEGIN

Display::Display()
{
	
}

void Display::run_shaders() const noexcept
{

}

void Display::set_data(const void* __restrict data_ptr,
                       const uint32_t data_width,
                       const uint32_t data_height,
                       const uint8_t data_type) noexcept
{
    const bool need_update = this->m_width != data_width ? true : 
                             this->m_height != data_height ? true :
                             this->m_data_type != data_type ? true : false;

    this->m_data = (void*)data_ptr;
    this->m_width = data_width;
    this->m_height = data_height;
    this->m_data_type = data_type;

    if(need_update)
    {
		spdlog::debug("[DISPLAY] : Updating GL display");

        glGenTextures(1, &this->m_gl_texture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, this->m_gl_texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 
					 0, 
					 DISPLAY_DATA_TYPE_TO_GL_INTERNAL_FMT(this->m_data_type), 
					 this->m_width, 
					 this->m_height, 
					 0, 
					 DISPLAY_DATA_TYPE_TO_GL_FMT(this->m_data_type), 
					 DISPLAY_DATA_TYPE_TO_GL_TYPE(this->m_data_type), 
					 this->m_data);

		glBindTexture(GL_TEXTURE_2D, 0);
    }
    else
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, this->m_gl_texture);
		glTexImage2D(GL_TEXTURE_2D, 
					 0, 
					 DISPLAY_DATA_TYPE_TO_GL_INTERNAL_FMT(this->m_data_type), 
					 this->m_width, 
					 this->m_height, 
					 0, 
					 DISPLAY_DATA_TYPE_TO_GL_FMT(this->m_data_type), 
					 DISPLAY_DATA_TYPE_TO_GL_TYPE(this->m_data_type), 
					 this->m_data);

		glBindTexture(GL_TEXTURE_2D, 0);
    }

	spdlog::debug("[DISPLAY] : Set data");
}

Display::~Display()
{
	if(this->m_gl_texture != GL_INVALID_VALUE)
	{
		glDeleteTextures(1, &this->m_gl_texture);
	}

	spdlog::debug("[DISPLAY] : Released Display");
}

LOV_NAMESPACE_END