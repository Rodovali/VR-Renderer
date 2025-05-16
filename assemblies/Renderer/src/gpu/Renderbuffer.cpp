// VR Renderer - Renderbuffer
// Rodolphe VALICON
// 2025

#include "Renderbuffer.h"

#include <utility>

namespace vr {
	namespace gpu {
	
		Renderbuffer::Renderbuffer() {
			glCreateRenderbuffers(1, &m_handle);
		}

		Renderbuffer::~Renderbuffer() {
			glDeleteRenderbuffers(1, &m_handle);
		}

		Renderbuffer::Renderbuffer(Renderbuffer&& other) noexcept
			: m_handle(std::exchange(other.m_handle, 0)) {}

		Renderbuffer& Renderbuffer::operator=(Renderbuffer&& other) noexcept {
			if (m_handle == other.m_handle) return *this;
			glDeleteRenderbuffers(1, &m_handle);

			m_handle = std::exchange(other.m_handle, 0);

			return *this;
		}

	}
}

