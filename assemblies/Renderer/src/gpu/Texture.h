// VR Renderer - Texture
// Rodolphe VALICON
// 2025

#pragma once

#include "gpu/Sampler.h"

#include <glad/glad.h>

namespace vr {
	namespace gpu {

		class Texture {
		public:
			Texture(GLenum type);
			Texture(GLenum type, const Sampler& sampler);
			~Texture();

			// No copy semantic
			Texture(const Texture&) = delete;
			Texture& operator=(const Texture&) = delete;

			// Move semantic
			Texture(Texture&& other) noexcept;
			Texture& operator=(Texture&& other) noexcept;

			inline operator GLuint() const { return m_handle; }

		private:
			GLuint m_handle;
		};

	}
}
