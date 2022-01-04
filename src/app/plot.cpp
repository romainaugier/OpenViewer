// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "plot.h"

namespace Interface
{
	namespace Plot
	{
		void Parade::Initialize() noexcept
		{
			// Generate the frame buffer object
			glGenFramebuffers(1, &this->m_FBO);

			// Generate the color attachement texture
			glGenTextures(1, &this->m_RenderTexture);
			glBindTexture(GL_TEXTURE_2D, this->m_RenderTexture);
			glTexImage2D(GL_TEXTURE_2D, 
						0, 
						GL_RGB8,
						this->m_Width, 
						this->m_Height, 
						0, 
						GL_RGB, 
						GL_UNSIGNED_BYTE, 
						nullptr);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			// Generate the render buffer object
			glGenRenderbuffers(1, &this->m_RBO);
			BindRBO();
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, this->m_Width, this->m_Height);

			// Attach the texture and the frame buffer
			BindFBO();
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->m_RenderTexture, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->m_RBO);

			// Verify that the framebuffer is complete
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
			{
				StaticErrorConsoleLog("[DISPLAY] : OPENGL Framebuffer is not complete.");
			}

			UnbindFBO();
			UnbindRBO();
		}

		void Parade::Update(const GLuint imageTextureID) noexcept
		{

		}

		OPENVIEWER_FORCEINLINE void Parade::BindFBO() const noexcept
		{
			glBindFramebuffer(GL_FRAMEBUFFER, this->m_FBO);
		}

		OPENVIEWER_FORCEINLINE void Parade::UnbindFBO() const noexcept
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		} 

		OPENVIEWER_FORCEINLINE void Parade::BindRBO() const noexcept
		{
			glBindRenderbuffer(GL_RENDERBUFFER, this->m_RBO);
		}

		OPENVIEWER_FORCEINLINE void Parade::UnbindRBO() const noexcept
		{
			glBindRenderbuffer(GL_RENDERBUFFER, this->m_RBO);
		}
	}
}