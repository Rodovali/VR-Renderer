// VR Renderer - Light
// Rodolphe VALICON
// 2025

#include "DirectionalLight.h"

#include <glm/gtc/matrix_transform.hpp>

namespace vr {

	void DirectionalLight::computeMatrix(const Camera& camera, float size) {
		
		glm::mat4 proj = glm::ortho(-size, size, -size, size, 0.1f, 75.0f);

		glm::vec3 targetPos = camera.eyePos + 2.0f * camera.forward;
		glm::vec3 viewPos = targetPos - 50.0f * glm::normalize(direction);

		glm::mat4 view = glm::lookAt(viewPos, targetPos, glm::vec3(0.0f, 1.0f, 0.0f));
		
		matrix = proj * view;
	}

}