// VR Renderer - Texture
// Rodolphe VALICON
// 2025

#include "Texture.h"

#include <utility>

namespace vr {
	namespace gpu {

		Texture::Texture(GLenum type) {
			glCreateTextures(type, 1, &m_handle);
		}

		Texture::Texture(GLenum type, const Sampler& sampler) {
			glCreateTextures(type, 1, &m_handle);

			// Set sampler settings
			glTextureParameteri(m_handle, GL_TEXTURE_MIN_FILTER, sampler.minFilter);
			glTextureParameteri(m_handle, GL_TEXTURE_MAG_FILTER, sampler.magFilter);
			glTextureParameteri(m_handle, GL_TEXTURE_WRAP_S, sampler.wrapS);
			glTextureParameteri(m_handle, GL_TEXTURE_WRAP_T, sampler.wrapT);
			glTextureParameteri(m_handle, GL_TEXTURE_WRAP_R, sampler.wrapR);
		}

		Texture::~Texture() {
			glDeleteTextures(1, &m_handle);
		}

		Texture::Texture(Texture&& other) noexcept
			: m_handle(std::exchange(other.m_handle, 0)) {}

		Texture& Texture::operator=(Texture&& other) noexcept {
			if (m_handle == other.m_handle) return *this;
			glDeleteTextures(1, &m_handle);

			m_handle = std::exchange(other.m_handle, 0);

			return *this;
		}

	}
}
