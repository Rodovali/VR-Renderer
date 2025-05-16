// VR Renderer - Vertex Layout
// Rodolphe VALICON
// 2025

#pragma once

#include <glad/glad.h>
#include <initializer_list>
#include <string_view>
#include <vector>
#include <unordered_map>

namespace vr {
	namespace gpu {

		/// @brief Vertex attribute "types "names".
		enum class Attribute : uint32_t {
			Position = 0,
			Normal = 1,
			Tangent = 2,
			TexCoord0 = 3,
			TexCoord1 = 4,
			Color = 5,
			Joints = 6,
			Weights = 7
		};

		/// @brief Vertex attribute, with its type, number of components, and offset.
		struct VertexAttribute {
			Attribute attribute;
			GLenum type;
			GLuint components;
			GLint offset;

			VertexAttribute() = default;

			/// @brief Instanciates a vertex attribute.
			/// The offset is initialized later by the vertex layout.
			/// @param attribute Attribute "name".
			/// @param type Attribute type (int, float, etc.).
			/// @param components Number of components (1 -> scalar, 2 -> vec2, etc.)
			VertexAttribute(Attribute attribute, GLenum type, GLuint components)
				: attribute(attribute), type(type), components(components), offset(0) {}
		};


		/// @brief Vertex layout descriptor.
		/// It's a collection of vertex attributes with auto-computed offsets and stride.
		class VertexLayout {
			using Attributes = std::unordered_map<Attribute, VertexAttribute>;

		public:
			/// @brief Initializes an empty layout.
			VertexLayout() = default;

			/// @brief Initializes a layout from a vector of attributes.
			/// @param layout std::vector of attributes.
			VertexLayout(const std::vector<VertexAttribute>& layout) { computeOffsets(layout); }

			/// @brief Initializes a layout from an initializer list of attributes.
			/// @param layout std::initialize_list of attributes.
			VertexLayout(std::initializer_list<VertexAttribute> layout) { computeOffsets(layout); }

			/// @brief Provides the stride of the layout.
			/// @return A 32 bit integer representing the stride of the layout.
			GLsizei getStride() const { return m_stride; }

			/// @brief Provides a named attribute
			/// @param Attribute "name" to fetch.
			/// @return A reference to the layout's vertex attribute.
			const VertexAttribute& getAttribute(Attribute attribute) const { return m_attributes.at(attribute); }

			void addAttribute(VertexAttribute attribute);

			bool hasAttribute(Attribute attribute) const { return m_attributes.find(attribute) != m_attributes.end(); }

			size_t size() const { return m_attributes.size(); }

			Attributes::iterator begin() { return m_attributes.begin(); }
			Attributes::iterator end() { return m_attributes.end(); }
			Attributes::const_iterator begin() const { return m_attributes.begin(); }
			Attributes::const_iterator end() const { return m_attributes.end(); }

		private:
			void computeOffsets(const std::vector<VertexAttribute>& attributes);
		private:
			Attributes m_attributes;
			GLsizei m_stride = 0;
		};

	}
}
