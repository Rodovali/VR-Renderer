// VR Renderer - Material Instance
// Rodolphe VALICON
// 2025

#include "MaterialInstance.h"

#include <imgui.h>
#include <glad/glad.h>

namespace vr {
	
	MaterialInstance::MaterialInstance(std::shared_ptr<Material> materialClass)
		: m_materialClass(materialClass) {
		// Allocate buffer
		m_bufferData = std::make_unique<uint8_t[]>(materialClass->getUniformBufferSize());
		m_buffer = gpu::Buffer(materialClass->getUniformBufferSize(), GL_DYNAMIC_DRAW);
	}

	void MaterialInstance::use() {
		// Update buffer data if needed
		if (m_dataPending) {
			glNamedBufferSubData(m_buffer, 0, m_materialClass->getUniformBufferSize(), m_bufferData.get());
			m_dataPending = false;
		}

		// Bind shader and uniform buffer
		glUseProgram(m_materialClass->getShaderProgram());
		glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_buffer);

		// Bind textures
		for (const auto& [slot, texture] : m_materialClass->getDefaultTextures()) {
			glBindTextureUnit(slot, *texture);
		}

		for (const auto& [slot, texture] : m_textures) {
			glBindTextureUnit(slot, *texture);
		}

		renderFlags.apply();
	}

	void MaterialInstance::immediateGUI() {
		bool modified = false;
		ImGui::PushID(this);
		for (auto& [name, uniform] : m_materialClass->getUniformLayout()) {
			switch (uniform.type) {
			case gpu::ShaderType::Bool:
				modified = ImGui::Checkbox(name.c_str(), reinterpret_cast<bool*>(&m_bufferData[uniform.offset]));
				break;
			case gpu::ShaderType::Int:
				modified = ImGui::DragInt(name.c_str(), reinterpret_cast<int*>(&m_bufferData[uniform.offset]), 0.01f);
				break;
			case gpu::ShaderType::Float:
				modified = ImGui::DragFloat(name.c_str(), reinterpret_cast<float*>(&m_bufferData[uniform.offset]), 0.01f);
				break;
			case gpu::ShaderType::Vec2:
				modified = ImGui::DragFloat2(name.c_str(), reinterpret_cast<float*>(&m_bufferData[uniform.offset]), 0.01f);
				break;
			case gpu::ShaderType::Color3:
				modified = ImGui::ColorEdit3(name.c_str(), reinterpret_cast<float*>(&m_bufferData[uniform.offset]));
				break;
			case gpu::ShaderType::Vec3:
				modified = ImGui::DragFloat3(name.c_str(), reinterpret_cast<float*>(&m_bufferData[uniform.offset]), 0.01f);
				break;
			case gpu::ShaderType::Color4:
				modified = ImGui::ColorEdit4(name.c_str(), reinterpret_cast<float*>(&m_bufferData[uniform.offset]));
				break;
			case gpu::ShaderType::Vec4:
				modified = ImGui::DragFloat4(name.c_str(), reinterpret_cast<float*>(&m_bufferData[uniform.offset]), 0.01f);
				break;
			case gpu::ShaderType::IVec2:
				modified = ImGui::DragInt2(name.c_str(), reinterpret_cast<int*>(&m_bufferData[uniform.offset]), 0.01f);
				break;
			case gpu::ShaderType::IVec3:
				modified = ImGui::DragInt3(name.c_str(), reinterpret_cast<int*>(&m_bufferData[uniform.offset]), 0.01f);
				break;
			case gpu::ShaderType::IVec4:
				modified = ImGui::DragInt4(name.c_str(), reinterpret_cast<int*>(&m_bufferData[uniform.offset]), 0.01f);
				break;
			default:
				break;
			}

			if (modified) m_dataPending = true;
		}
		ImGui::PopID();
	}

}
