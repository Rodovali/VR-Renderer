// VR Renderer - Vertex Layout
// Rodolphe VALICON
// 2025

#include "VertexLayout.h"

#include "core/Logger.h"

static GLsizei getTypeSize(GLenum type) {
	switch (type) {
	case GL_BYTE:
	case GL_UNSIGNED_BYTE:	return 1;
	case GL_SHORT:
	case GL_UNSIGNED_SHORT:
	case GL_HALF_FLOAT:		return 2;
	case GL_INT:
	case GL_UNSIGNED_INT:
	case GL_FLOAT:			return 4;
	case GL_DOUBLE:			return 8;
	default:				return 0;
	}
}

namespace vr {
	namespace gpu {
		
		void VertexLayout::computeOffsets(const std::vector<VertexAttribute>& layout) {
			for (VertexAttribute attribute : layout) {
				attribute.offset = m_stride;
				m_attributes[attribute.attribute] = attribute;
				m_stride += getTypeSize(attribute.type) * attribute.components;
			}
		}

		void VertexLayout::addAttribute(VertexAttribute attribute) {
			attribute.offset = m_stride;
			m_attributes[attribute.attribute] = attribute;
			m_stride += getTypeSize(attribute.type) * attribute.components;
		}
	}
}
