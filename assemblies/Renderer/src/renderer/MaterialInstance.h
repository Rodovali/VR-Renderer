// VR Renderer - Material Instance
// Rodolphe VALICON
// 2025

#pragma once

#include "core/Logger.h"
#include "gpu/Buffer.h"
#include "gpu/ShaderType.h"
#include "renderer/Material.h"
#include "renderer/RenderFlags.h"
#include "utils/Macros.h"

#include <memory>

namespace vr {

	class MaterialInstance {
	public:
		MaterialInstance() = default;
		MaterialInstance(std::shared_ptr<Material> materialClass);

		template<typename T>
		T get(const std::string& name) const {
			const Material::Uniform* uniform = m_materialClass->getUniformInfo(name);
			if (!uniform) {
				logger::error("Uniform '{}' not found.", name);
				return nullptr;
			}

			gpu::ShaderType requestedType = gpu::ShaderType::associated_type<T>();
			gpu::ShaderType actualType = uniform->type;
			if ((requestedType & actualType) == 0) {
				logger::error("Uniform '{}' is of type {}, not {}", name, actualType.name(), requestedType.name());
				return nullptr;
			}

			T result;
			std::memcpy(&result, &m_bufferData[uniform->offset], sizeof(T));
			return result;
		}

		template<typename T>
		void set(const std::string& name, T value) {
			const Material::Uniform* uniform = m_materialClass->getUniformInfo(name);
			if (!uniform) {
				logger::error("Uniform '{}' not found.", name);
				return;
			}

			gpu::ShaderType requestedType = gpu::ShaderType::associated_type<T>();
			gpu::ShaderType actualType = uniform->type;
			if ((requestedType & actualType) == 0) {
				logger::error("Uniform '{}' is of type {}, not {}", name, actualType.name(), requestedType.name());
				return;
			}

			std::memcpy(&m_bufferData[uniform->offset], &value, sizeof(T));
			m_dataPending = true;
		}

		void setTexture(const std::string& name, std::shared_ptr<gpu::Texture> texture) {
			uint32_t slot = m_materialClass->getTextureSlot(name);
			if (slot == -1) {
				logger::error("Texture '{}' not found.", name);
				return;
			}

			m_textures[slot] = texture;
		}

		void use();
		void immediateGUI();
	public:
		RenderFlags renderFlags;
	private:
		std::shared_ptr<Material> m_materialClass;
		std::unique_ptr<uint8_t[]> m_bufferData;
		gpu::Buffer m_buffer;

		bool m_dataPending = false;

		std::unordered_map<GLuint, std::shared_ptr<gpu::Texture>> m_textures;
	};

}
