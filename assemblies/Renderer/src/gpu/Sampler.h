// VR Renderer - Texture Sampler
// Rodolphe VALICON
// 2025

#pragma once

#include <glad/glad.h>

namespace vr {
	namespace gpu {
		
		/// @brief Represents the sampler state of a texture.
		struct Sampler {
			GLint magFilter = GL_LINEAR;
			GLint minFilter = GL_LINEAR_MIPMAP_LINEAR;
			GLint wrapS = GL_REPEAT;
			GLint wrapT = GL_REPEAT;
			GLint wrapR = GL_REPEAT;
		};

	}
}
