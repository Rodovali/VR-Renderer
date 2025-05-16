// VR Renderer - Framebuffer
// Rodolphe VALICON
// 2025

#pragma once

#include <glad/glad.h>

namespace vr {
	namespace gpu {
		
		class Framebuffer {
		public:
			Framebuffer();
			~Framebuffer();

			// No copy semantic
			Framebuffer(const Framebuffer&) = delete;
			Framebuffer& operator=(const Framebuffer&) = delete;

			// Move semantic
			Framebuffer(Framebuffer&& other) noexcept;
			Framebuffer& operator=(Framebuffer&& other) noexcept;

			inline operator GLuint() const { return m_handle; }

		private:
			GLuint m_handle;
		};

	}
}
