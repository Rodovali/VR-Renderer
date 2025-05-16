// VR Renderer - Scene
// Rodolphe VALICON
// 2025

#pragma once

#include "renderer/Skybox.h"
#include "renderer/Mesh.h"
#include "renderer/DirectionalLight.h"
#include "renderer/PointLight.h"

#include <memory>
#include <vector>


namespace vr {

	struct Scene {
		std::shared_ptr<Skybox> skybox;
		std::vector<std::shared_ptr<Mesh>> meshes;
		std::vector<DirectionalLight> directionalLights;
		std::vector<PointLight> pointLights;
	};

}