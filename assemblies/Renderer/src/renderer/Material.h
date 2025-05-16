// VR Renderer - Material System
// Rodolphe VALICON
// 2025

#pragma once

#include "gpu/ShaderProgram.h"
#include "gpu/ShaderType.h"
#include "gpu/Texture.h"

#include <glad/glad.h>

#include <cstdint>
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>

namespace vr {
	
	class Material {
	public:
		struct UniformDescriptor {
			std::string name;
			gpu::ShaderType type;
		};

		struct TextureDescriptor {
			std::string name;
			GLuint slot;
		};

		struct Uniform {
			gpu::ShaderType type;
			size_t offset;
		};

	public:
		Material() = default;
		Material(const char* shaderPath);

		void setUniformLayout(const std::vector<UniformDescriptor>& layout);
		void setTextureLayout(const std::vector<TextureDescriptor>& layout);

		void setDefaultTexture(GLuint slot, std::shared_ptr<gpu::Texture> texture) { m_defaultTextures[slot] = texture; }

		gpu::ShaderProgram& getShaderProgram() const { return *m_shader; }
		const Uniform* getUniformInfo(const std::string& name) const;
		int32_t getTextureSlot(const std::string& name) const;

		uint32_t getUniformBufferSize() const { return m_uniformBufferSize; }

		const std::unordered_map<std::string, Uniform>& getUniformLayout() const { return m_uniformLayout; }
		const std::unordered_map<std::string, GLuint>& getTextureLayout() const { return m_textureLayout; }
		const std::unordered_map<GLuint, std::shared_ptr<gpu::Texture>>& getDefaultTextures() const { return m_defaultTextures; }

		void reload() { m_shader->reload(); }

		static std::shared_ptr<Material> loadFromJSON(const std::string& path);

	private:
		std::unique_ptr<gpu::ShaderProgram> m_shader;
		std::unordered_map<std::string, Uniform> m_uniformLayout;
		std::unordered_map<std::string, GLuint> m_textureLayout;
		std::unordered_map<GLuint, std::shared_ptr<gpu::Texture>> m_defaultTextures;

		uint32_t m_uniformBufferSize;
	};

}
