// VR Renderer - GLTF 2.0 Loader
// Rodolphe VALICON
// 2025

#pragma once

#include "renderer/Mesh.h"

#include <memory>
#include <string>

namespace vr {
	namespace utils {
		/// @brief glTF 2.0 3D model loader.
		/// @param filePath Path to GLTF file.
		/// @param meshIndex Index of the mesh to load.
		/// @return A shared pointer to the loaded model.
		std::shared_ptr<Mesh> loadGLTFMesh(const std::string& filePath, uint32_t meshIndex);
	}
}