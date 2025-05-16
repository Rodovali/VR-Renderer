// VR Renderer - Render Target
// Rodolphe VALICON
// 2025

#pragma once

#include "gpu/Framebuffer.h"
#include "gpu/Texture.h"
#include "gpu/Renderbuffer.h"

#include <cstdint>
#include <memory>

namespace vr {

	class RenderTarget {
	public:
		RenderTarget(int32_t width, int32_t height, int32_t samples = 1);

		const gpu::Framebuffer& getFramebuffer() const { return m_framebuffer; }
		std::shared_ptr<gpu::Texture> getColorTexture() const { return m_color; }
		std::shared_ptr<gpu::Renderbuffer> getDepthStencilBuffer() const { return m_depthStencil; }

		int32_t getWidth() const { return m_width; }
		int32_t getHeight() const { return m_height; }

	private:
		gpu::Framebuffer m_framebuffer;
		std::shared_ptr<gpu::Texture> m_color;
		std::shared_ptr<gpu::Renderbuffer> m_depthStencil;

		int32_t m_width;
		int32_t m_height;
	};

}