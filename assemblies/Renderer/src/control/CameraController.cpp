// VR Renderer - Camera Controller
// Rodolphe VALICON
// 2025

#include "CameraController.h"

#include "core/Input.h"

#include <glm/ext.hpp>
#include <GLFW/glfw3.h>

namespace vr {
	
	CameraController::CameraController() : m_direction(0.0f), m_angularVelocity(0.0f) {}

	void CameraController::update(Camera& camera, float deltaTime) {
		// Extract camera basis
		const glm::vec3 forward = glm::normalize(camera.forward);
		const glm::vec3 right = glm::normalize(glm::cross(forward, camera.up));
		const glm::vec3 up = glm::cross(right, forward);

		// Translate camera
		const glm::vec3 vel = right * m_direction.x + up * m_direction.y + forward * m_direction.z;
		camera.eyePos += vel * speed * deltaTime;
		m_direction = glm::vec3(0.0f);

		// Rotate camera
		// NOTE: Mouse movement is already proportional to frame time
		const glm::quat upRotation = glm::angleAxis(glm::radians(-m_angularVelocity.x / 10.0f), camera.up);
		camera.forward = upRotation * forward;
		const glm::vec3 newRight = glm::normalize(glm::cross(camera.forward, camera.up));
		const glm::quat rightRotation = glm::angleAxis(glm::radians(-m_angularVelocity.y / 10.0f), newRight);

		// - Only apply right rotation if angle is still in bound.
		const glm::vec3 newForward = rightRotation * camera.forward;
		if (glm::abs(glm::dot(newForward, camera.up)) < 0.99) {
			camera.forward = newForward;
		}
	}
	
	bool CameraController::handle_input() {
		const glm::vec3 unitX(1.0f, 0.0f, 0.0f);
		const glm::vec3 unitY(0.0f, 1.0f, 0.0f);
		const glm::vec3 unitZ(0.0f, 0.0f, 1.0f);

		// Handle Keyboarad
		if (input::isKeyDown(GLFW_KEY_W)) {
			m_direction += unitZ;
		}
		if (input::isKeyDown(GLFW_KEY_S)) {
			m_direction -= unitZ;
		}
		if (input::isKeyDown(GLFW_KEY_D)) {
			m_direction += unitX;
		}
		if (input::isKeyDown(GLFW_KEY_A)) {
			m_direction -= unitX;
		}
		if (input::isKeyDown(GLFW_KEY_SPACE)) {
			m_direction += unitY;
		}
		if (input::isKeyDown(GLFW_KEY_LEFT_SHIFT)) {
			m_direction -= unitY;
		}

		// Handle Mouse
		if (input::isButtonDown(MouseButton::Left)) {
			input::captureMouse(true);
			input::getMouseDelta(m_angularVelocity.x, m_angularVelocity.y);
		} else {
			input::captureMouse(false);
			m_angularVelocity = glm::vec2(0.0f);
		}

		return (glm::length(m_direction) > 0.0f) || (glm::length(m_angularVelocity) > 0.0f);
	}

}