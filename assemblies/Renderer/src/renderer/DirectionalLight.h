// VR Renderer - Directional Light
// Rodolphe VALICON
// 2025

#pragma once

#include "renderer/Camera.h"

#include <glm/glm.hpp>

namespace vr {

	struct DirectionalLight {
		alignas(16) glm::vec3 direction;
		alignas(16) glm::vec3 color;
		float power;
		alignas(16) glm::mat4 matrix;

		void computeMatrix(const Camera& camera, float size = 5.0f);
	};

}