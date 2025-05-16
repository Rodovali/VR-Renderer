// VR Renderer - Transform
// Rodolphe VALICON
// 2025

#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace vr {

	struct Transform {
		glm::vec3 scale;
		glm::quat rotation;
		glm::vec3 translation;

		Transform()
			: scale(1.0f), rotation({ 0.0f, 0.0f, 0.0f }), translation(0.0f) 
		{}

		glm::mat4 getModelMatrix() const {
			glm::mat4 mat(1.0f);
			mat = glm::scale(mat, scale);
			mat = glm::mat4_cast(rotation) * mat;
			mat = glm::translate(glm::mat4(1.0f), translation) * mat;

			return mat;
		}

		glm::mat4 getNormalMatrix() const {
			glm::mat4 mat(1.0f);
			mat = glm::scale(mat, 1.0f / scale);
			mat = glm::mat4_cast(rotation) * mat;

			return mat;
		}

	};
}