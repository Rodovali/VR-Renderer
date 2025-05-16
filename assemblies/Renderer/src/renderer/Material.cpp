// VR Renderer - Material System
// Rodolphe VALICON
// 2025

#include "Material.h"

#include "core/Logger.h"

#include <nlohmann/json.hpp>

#include <fstream>

namespace vr {

	Material::Material(const char* shaderPath)
		: m_shader(std::make_unique<gpu::ShaderProgram>(shaderPath)), m_uniformBufferSize(0) {}

	void Material::setUniformLayout(const std::vector<Material::UniformDescriptor>& layout) {
		// Compute uniform offsets
		uint32_t currentOffset = 0;
		logger::debug("Constructing material uniform block layout:");
		for (const UniformDescriptor& descriptor : layout) {
			uint32_t alignment = descriptor.type.alignment();
			if (currentOffset % alignment != 0)
				currentOffset += alignment - currentOffset % alignment;

			m_uniformLayout[descriptor.name] = { descriptor.type, currentOffset };
			logger::debug("\t{:<3} - {} ({})", currentOffset, descriptor.type.name(), descriptor.name);
			currentOffset += descriptor.type.size();
		}
		logger::debug("Total size: {} bytes", currentOffset);
		m_uniformBufferSize = currentOffset;
	}

	void Material::setTextureLayout(const std::vector<Material::TextureDescriptor>& layout) {
		for (const TextureDescriptor& descriptor : layout) {
			m_textureLayout[descriptor.name] = descriptor.slot;
		}
	}

	const Material::Uniform* Material::getUniformInfo(const std::string& name) const {
		auto uniform = m_uniformLayout.find(name);
		if (uniform == m_uniformLayout.end())
			return nullptr;

		return &uniform->second;
	}

	int32_t Material::getTextureSlot(const std::string& name) const {
		auto slot = m_textureLayout.find(name);
		if (slot == m_textureLayout.end())
			return -1;

		return slot->second;
	}

	std::shared_ptr<Material> Material::loadFromJSON(const std::string& path) {
		using namespace nlohmann;
		
		// Load JSON data
		json content;
		std::ifstream matFile(path);
		matFile >> content;

		auto material = std::make_shared<Material>(static_cast<std::string>(content["shader"]).c_str());
		
		// Extract uniform layout
		const json& uniformList = content["uniforms"];
		std::vector<UniformDescriptor> uniformLayout;
		uniformLayout.reserve(uniformList.size());
		for (const json& uniform : uniformList) {
			uniformLayout.push_back({ uniform["name"], gpu::ShaderType::fromName(uniform["type"]) });
		}
		material->setUniformLayout(uniformLayout);

		// Extract texture layout
		const json& textureList = content["textures"];
		std::vector<TextureDescriptor> textureLayout;
		textureLayout.reserve(textureList.size());
		for (const json& texture : textureList) {
			textureLayout.push_back({ texture["name"], texture["slot"] });
		}
		material->setTextureLayout(textureLayout);

		logger::info("Loaded material '{}'.", static_cast<std::string>(content["name"]));


		return material;
	}

}
