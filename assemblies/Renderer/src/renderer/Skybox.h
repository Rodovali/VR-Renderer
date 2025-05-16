// VR Renderer - Skybox
// Rodolphe VALICON
// 2025

#pragma once

#include "renderer/Primitive.h"

#include <string>
#include <memory>

namespace vr {

	struct Skybox : public Primitive {
		Skybox(const std::string& path, float exposureCorrection = 1.0f);

		gpu::Texture& getCubeMap() { return *m_cubeMap; }

		std::unique_ptr<gpu::Texture> m_cubeMap;
	};

}