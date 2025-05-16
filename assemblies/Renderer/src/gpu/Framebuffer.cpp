// VR Renderer - Framebuffer
// Rodolphe VALICON
// 2025

#include "Framebuffer.h"

#include <utility>

namespace vr {
	namespace gpu {
		
		Framebuffer::Framebuffer() {
			glCreateFramebuffers(1, &m_handle);
		}

		Framebuffer::~Framebuffer() {
			glDeleteFramebuffers(1, &m_handle);
		}

		Framebuffer::Framebuffer(Framebuffer&& other) noexcept
			: m_handle(std::exchange(other.m_handle, 0)) {}

		Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept {
			if (m_handle == other.m_handle) return *this;
			glDeleteFramebuffers(1, &m_handle);

			m_handle = std::exchange(other.m_handle, 0);

			return *this;
		}

	}
}
