// VR Renderer - Render Target
// Rodolphe VALICON
// 2025

#include "RenderTarget.h"

#include <glad/glad.h>

namespace vr {

	RenderTarget::RenderTarget(int32_t width, int32_t height, int32_t samples)
		: m_width(width), m_height(height)
	{
		if (samples == 1) {
			m_color = std::make_shared<gpu::Texture>(GL_TEXTURE_2D);
			glTextureStorage2D(*m_color, 1, GL_RGBA16F, width, height);
			glTextureParameteri(*m_color, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(*m_color, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTextureParameteri(*m_color, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(*m_color, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			m_depthStencil = std::make_shared<gpu::Renderbuffer>();
			glNamedRenderbufferStorage(*m_depthStencil, GL_DEPTH24_STENCIL8, width, height);
		} else {
			m_color = std::make_shared<gpu::Texture>(GL_TEXTURE_2D_MULTISAMPLE);
			glTextureStorage2DMultisample(*m_color, samples, GL_RGBA16F, width, height, GL_TRUE);

			m_depthStencil = std::make_shared<gpu::Renderbuffer>();
			glNamedRenderbufferStorageMultisample(*m_depthStencil, samples, GL_DEPTH24_STENCIL8, width, height);
		}

		m_framebuffer = gpu::Framebuffer();
		glNamedFramebufferTexture(m_framebuffer, GL_COLOR_ATTACHMENT0, *m_color, 0);
		glNamedFramebufferRenderbuffer(m_framebuffer, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, *m_depthStencil);
	}

}