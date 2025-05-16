// VR Renderer - GPU Buffer
// Rodolphe VALICON
// 2025

#include "Buffer.h"

#include <utility>

namespace vr {
	namespace gpu {

		Buffer::Buffer(size_t size, GLenum usage, const uint8_t* data) {
			glCreateBuffers(1, &m_handle);
			glNamedBufferData(m_handle, size, data, usage);
		}

		Buffer::Buffer(size_t size, GLenum usage) {
			glCreateBuffers(1, &m_handle);
			glNamedBufferData(m_handle, size, nullptr, usage);
		}

		Buffer::Buffer(Buffer&& other) noexcept
			: m_handle(std::exchange(other.m_handle, 0)) {}

		Buffer& Buffer::operator=(Buffer&& other) noexcept {
			if (m_handle == other.m_handle) return *this;
			glDeleteBuffers(1, &m_handle);

			m_handle = std::exchange(other.m_handle, 0);

			return *this;
		}

		Buffer::~Buffer() {
			glDeleteBuffers(1, &m_handle);
		}

	}
}

