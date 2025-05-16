// VR Renderer - Vertex Array
// Rodolphe VALICON
// 2025

#include "VertexArray.h"

namespace vr {
	namespace gpu {
		
		VertexArray::VertexArray(const GeometryData& geometry) {
			glCreateVertexArrays(1, &m_handle);
			setLayout(geometry.layout);
			m_vertexBuffer = Buffer(geometry.vertex_data.size(), GL_STATIC_DRAW, geometry.vertex_data.data());
			m_elementBuffer = Buffer(geometry.indices.size() * sizeof(uint32_t), GL_STATIC_DRAW, reinterpret_cast<const uint8_t*>(geometry.indices.data()));
			m_elementCount = static_cast<uint32_t>(geometry.indices.size());
			m_topology = geometry.topology;

			glVertexArrayVertexBuffer(m_handle, 0, m_vertexBuffer, 0, geometry.layout.getStride());
			glVertexArrayElementBuffer(m_handle, m_elementBuffer);
		}

		VertexArray::VertexArray(VertexArray&& other) noexcept :
			m_handle(std::exchange(other.m_handle, 0)),
			m_vertexBuffer(std::move(other.m_vertexBuffer)),
			m_elementBuffer(std::move(other.m_elementBuffer)),
			m_elementCount(std::exchange(other.m_elementCount, 0)),
			m_topology(std::exchange(other.m_topology, 0))
		{}

		VertexArray& VertexArray::operator=(VertexArray&& other) noexcept {
			if (m_handle == other.m_handle) return *this;
			glDeleteVertexArrays(1, &m_handle);

			m_handle = std::exchange(other.m_handle, 0);
			m_vertexBuffer = std::move(other.m_vertexBuffer);
			m_elementBuffer = std::move(other.m_elementBuffer);
			m_elementCount = std::exchange(other.m_elementCount, 0);
			m_topology = std::exchange(other.m_topology, 0);

			return *this;
		}

		VertexArray::~VertexArray() {
			glDeleteVertexArrays(1, &m_handle);
		}

		void VertexArray::setLayout(const VertexLayout& layout) const {
			for (const auto& [_, attribute] : layout) {
				uint32_t index = static_cast<uint32_t>(attribute.attribute);
				glEnableVertexArrayAttrib(m_handle, index);
				glVertexArrayAttribBinding(m_handle, index, 0);
				glVertexArrayAttribFormat(m_handle, index, attribute.components, attribute.type, GL_FALSE, attribute.offset);
			}
		}

	}
}
