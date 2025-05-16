// VR Renderer - GPU Buffer
// Rodolphe VALICON
// 2025

#pragma once

#include <glad/glad.h>

#include <cstdint>

namespace vr {
	namespace gpu {

		// A memory buffer, allocated on the GPU.
		class Buffer {
		public:

			/// @brief Creates a decoy invalid buffer.
			Buffer() : m_handle(0) {}

			/// @brief Allocates and initializes a buffer on the GPU.
			/// @param size Buffer size to allocate, in bytes.
			/// @param usage Buffer usage hint, for optimized memory placement.
			/// @param data Pointer to the data to initialize the buffer with.
			Buffer(size_t size, GLenum usage, const uint8_t* data);

			/// @brief Allocates a buffer on the GPU.
			/// @param size Buffer size to allocate, in bytes.
			/// @param usage Buffer usage hint, for optimized memory placement.
			Buffer(size_t size, GLenum usage);

			// No copy semantics
			Buffer(const Buffer&) = delete;
			Buffer& operator=(const Buffer&) = delete;

			// Move semantics
			Buffer(Buffer&& other) noexcept;
			Buffer& operator=(Buffer&& other) noexcept;

			~Buffer();

			/// @brief Transparently casts the buffer object into its GL handle.
			inline operator GLuint() const { return m_handle; }
		private:
			GLuint m_handle;
		};

	}
}
