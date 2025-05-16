// VR Renderer - Camera Controller
// Rodolphe VALICON
// 2025

#pragma once

#include "renderer/Camera.h"

#include <glm/glm.hpp>

namespace vr {

	class CameraController {
	public:
		CameraController();

		void update(Camera& camera, float deltaTime);
		bool handle_input();

	public:
		float speed = 2.0f;
	private:
		glm::vec3 m_direction;
		glm::vec2 m_angularVelocity;
	};

}