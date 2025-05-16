// VR Renderer - Geometry Data
// Rodolphe VALICON
// 2025

#pragma once

#include "gpu/VertexLayout.h"

#include <glad/glad.h>

#include <cstdint>
#include <vector>

namespace vr{
	namespace gpu {
		
		/// @brief Contains the data required to construct an indexed mesh.
		struct GeometryData {
			VertexLayout layout;
			std::vector<uint8_t> vertex_data;
			std::vector<uint32_t> indices;
			GLenum topology = 0;
		};

	}
}
