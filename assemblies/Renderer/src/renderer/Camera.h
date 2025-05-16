// VR Renderer - Camera
// Rodolphe VALICON
// 2025

#pragma once

#include <glm/glm.hpp>

namespace vr {

	class Camera {
	public:
		Camera();

		glm::mat4 getViewMatrix() const;
		glm::mat4 getProjectionMatrix() const;
	public:
		glm::vec3 eyePos;
		glm::vec3 forward;
		glm::vec3 up;
		float aspect;
		float fovy;
		float zNear;
		float zFar;
	};

}