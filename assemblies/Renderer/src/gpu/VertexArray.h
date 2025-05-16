// VR Renderer - Vertex Array
// Rodolphe VALICON
// 2025

#pragma once

#include <glad/glad.h>

#include "gpu/Buffer.h"
#include "gpu/GeometryData.h"
#include "gpu/VertexLayout.h"


#include <cstdint>
#include <vector>

namespace vr {
	namespace gpu {

		class VertexArray {
		public:
			VertexArray() = default;
			VertexArray(const GeometryData& geometry);

			// No copy semantic
			VertexArray(const VertexArray&) = delete;
			VertexArray& operator=(const VertexArray&) = delete;

			// Move semantic
			VertexArray(VertexArray&& other) noexcept;
			VertexArray& operator=(VertexArray&& other) noexcept;

			~VertexArray();

			operator GLuint() { return m_handle; }

			uint32_t getElementCount() const { return m_elementCount; }
			GLenum getTopology() const { return m_topology; }

		private:
			void setLayout(const VertexLayout& layout) const;

		private:
			GLuint m_handle;

			Buffer m_vertexBuffer;
			Buffer m_elementBuffer;

			GLenum m_topology;
			uint32_t m_elementCount;
		};

	}
}
