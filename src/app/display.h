// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#pragma once

#include "imgui.h"

#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui_internal.h"

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "utils/gl_utils.h"

#include "OpenEXR/half.h"
#include <immintrin.h>

#include "scopes.h"
#include "implaybar.h"
#include "core/loader.h"
#include "core/ocio.h"
#include "utils/profiler.h"
#include "utils/memory/alloc.h"

namespace Interface
{
	struct Display
	{
		ImPlaybar m_Playbar;

		Profiler* m_Profiler;

		Logger* m_Logger;
		
		Core::Loader* m_Loader;

		ImVec2 m_HoverCoordinates = ImVec2(0.0f, 0.0f);
		ImVec2 m_DisplayPos = ImVec2(0.0f, 0.0f);
		ImVec2 m_OldMousePos = ImVec2(0.0f, 0.0f);

		double m_LastTimeActive = 0;

		Utils::GL::Shader m_AlphaBlendingShader;

		GLuint m_RawTexture;
		GLuint m_TransformedTexture;
		GLuint m_DisplayTexture;
		GLuint m_FBO, m_RBO;

		int32_t m_MediaID = -1;
		int32_t m_BackGroundMode = 0;

		float m_Zoom = 1.0f;
		
		uint16_t m_Width;
		uint16_t m_Height;

		uint8_t m_DisplayID = 0;

		bool m_NeedReinitialization = false;
		bool m_IsOpen = true;
		bool m_IsActive = false;
		bool m_IsImageHovered = false;
		bool m_FrameView = false;
		bool m_HomeView = false;
		bool m_HorizontalMirrorView = false;
		bool m_VerticalMirrorView = false;
		bool m_PremultiplyAlpha = false;

		Display(Profiler* profiler, Logger* logger, Core::Loader* loader, const uint8_t id)
		{
			this->m_Profiler = profiler;
			this->m_Logger = logger;
			this->m_Loader = loader;
			this->m_DisplayID = id;
		}

		void Initialize(Core::Ocio& ocio, const uint32_t mediaId) noexcept;

		void ReInitialize(const Core::Image& image, Core::Ocio& ocio, const uint32_t mediaId) noexcept;

		OV_FORCEINLINE void NeedReinit(const bool need = true) noexcept { this->m_NeedReinitialization = need; }
		
		// Few OpenGL utilities
		void InitializeOpenGL(const Core::Image& image) noexcept;
		OV_FORCEINLINE void BindFBO() const noexcept { glBindFramebuffer(GL_FRAMEBUFFER, this->m_FBO); }
		OV_FORCEINLINE void UnbindFBO() const noexcept { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
		OV_FORCEINLINE void BindRBO() const noexcept { glBindRenderbuffer(GL_RENDERBUFFER, this->m_RBO); }
		OV_FORCEINLINE void UnbindRBO() const noexcept { glBindRenderbuffer(GL_RENDERBUFFER, 0); }
		void InitAlphaBlendingTexture() noexcept;
		void InitRawTexture(const Core::Image* initImage) noexcept;
		void OcioTransform(Core::Ocio& ocio, const bool updateProcessor = true) noexcept;
		void AlphaBlending() noexcept;
		
		// Updates the displayed image
		void Update(Core::Ocio& ocio) noexcept;

		// Few functions to play with the displayed image
		OV_FORCEINLINE void NeedFrame() noexcept { this->m_FrameView = true; }
		OV_FORCEINLINE void NeedHome() noexcept { this->m_HomeView = true; }
		OV_FORCEINLINE void MirrorHorizontal() noexcept { this->m_HorizontalMirrorView = !this->m_HorizontalMirrorView; }
		OV_FORCEINLINE void MirrorVertical() noexcept { this->m_VerticalMirrorView = !this->m_VerticalMirrorView; }

		// Few getters/setters
		OV_FORCEINLINE double GetLastTimeActive() const noexcept { return this->m_LastTimeActive; }
		void SetMedia(const uint32_t mediaId) noexcept;
		OV_FORCEINLINE int32_t GetMediaId() const noexcept { return this->m_MediaID; }
		OV_FORCEINLINE ImPlaybar* AssociatedPlaybar() noexcept { return &this->m_Playbar; }

		// Associated playbar utilities
		OV_FORCEINLINE void UpdateAssociatedPlaybar() noexcept { this->m_Playbar.Update(); }
		
		// Display window draw function
		void Draw() noexcept;
		
		// Retrieves a pixel from the current transformed texture
		ImVec4 GetPixel(const uint16_t x, const uint16_t y) const noexcept;
		
		void Release() noexcept;
	};
} // End namespace Interface