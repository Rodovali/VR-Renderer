// VR Renderer - Renderbuffer
// Rodolphe VALICON
// 2025

#pragma once

#include <glad/glad.h>

namespace vr {
	namespace gpu {
	
		class Renderbuffer {
		public:
			Renderbuffer();
			~Renderbuffer();

			// No copy semantic
			Renderbuffer(const Renderbuffer&) = delete;
			Renderbuffer& operator=(const Renderbuffer&) = delete;

			// Move semantic
			Renderbuffer(Renderbuffer&& other) noexcept;
			Renderbuffer& operator=(Renderbuffer&& other) noexcept;

			inline operator GLuint() const { return m_handle; }

		private:
			GLuint m_handle;
		};

	}
}

