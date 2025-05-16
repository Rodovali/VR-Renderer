#pragma once
// VR Renderer - Directional Light
// Rodolphe VALICON
// 2025

#pragma once

#include "renderer/Camera.h"

#include <glm/glm.hpp>

namespace vr {

	struct PointLight {
		alignas(16) glm::vec3 position;
		alignas(16) glm::vec3 color;
		float power;
		float radius;
	};

}