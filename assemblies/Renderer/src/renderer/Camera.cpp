// VR Renderer - Camera
// Rodolphe VALICON
// 2025

#include "Camera.h"

#include <glm/ext.hpp>

namespace vr {

	Camera::Camera()
		: eyePos({ 0.0f, 0.5f, 0.5f }), forward({ 0.0f, 0.0f, -1.0f }),
		up({ 0.0f, 1.0f, 0.0f }), aspect(16.0f / 9.0f), fovy(90.0f),
		zNear(0.1f), zFar(100.0f)
	{}

	glm::mat4 Camera::getViewMatrix() const {
		return glm::lookAtRH(eyePos, eyePos + forward, up);
	}

	glm::mat4 Camera::getProjectionMatrix() const {
		return glm::perspective(glm::radians(fovy), aspect, zNear, zFar);
	}

}