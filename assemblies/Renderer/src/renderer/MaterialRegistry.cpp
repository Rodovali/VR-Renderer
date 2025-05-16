// VR Renderer - Material Registry
// Rodolphe VALICON
// 2025

#include "MaterialRegistry.h"

#include "core/Logger.h"
#include "gpu/Texture.h"
#include "gpu/VertexArray.h"
#include "gpu/Renderbuffer.h"
#include "gpu/Framebuffer.h"

#include <filesystem>

static std::shared_ptr<vr::gpu::Texture> ComputeCookTorranceLUT(uint32_t size) {
	using namespace vr;
	// Prepare rendering quad
	const float vertices[] = {
		-1.0f, -1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 1.0f, 0.0f,
		 1.0f,  1.0f, 1.0f, 1.0f,
		-1.0f,  1.0f, 0.0f, 1.0f
	};
	std::vector<uint8_t> quadData(sizeof(vertices));
	std::memcpy(quadData.data(), vertices, sizeof(vertices));
	std::vector<uint32_t> quadIndices{
		0, 1, 2, 2, 3, 0
	};

	gpu::VertexLayout quadLayout{
		{ gpu::Attribute::Position, GL_FLOAT, 2 },
		{ gpu::Attribute::TexCoord0, GL_FLOAT, 2 }
	};

	gpu::GeometryData geometry;
	geometry.indices = quadIndices;
	geometry.vertex_data = quadData;
	geometry.layout = quadLayout;
	geometry.topology = GL_TRIANGLES;

	gpu::VertexArray quad(geometry);

	// Prepare LUT
	std::shared_ptr<gpu::Texture> LUT = std::make_shared<gpu::Texture>(GL_TEXTURE_2D);
	glTextureParameteri(*LUT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(*LUT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(*LUT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(*LUT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureStorage2D(*LUT, 1, GL_RG32F, size, size);

	// Prepare rendering framebuffer
	gpu::Renderbuffer depthBuffer;
	glNamedRenderbufferStorage(depthBuffer, GL_DEPTH_COMPONENT24, size, size);

	gpu::Framebuffer framebuffer;
	glNamedFramebufferRenderbuffer(framebuffer, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
	glNamedFramebufferTexture(framebuffer, GL_COLOR_ATTACHMENT0, *LUT, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glViewport(0, 0, size, size);

	gpu::ShaderProgram cookTorranceLUTShader("res/shaders/pre-render/cook-torrance_LUT.glsl");
	glUseProgram(cookTorranceLUTShader);
	glBindTextureUnit(0, *LUT);
	glBindVertexArray(quad);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

	return LUT;
}

namespace vr {
	std::unordered_map<std::string, std::shared_ptr<Material>> MaterialRegistry::s_materials;

	void MaterialRegistry::registerMaterial(std::string name, std::shared_ptr<Material> material) {
		if (s_materials.find(name) == s_materials.end()) {
			s_materials[name] = material;
		} else {
			logger::warn("Tried to register a material with an already taken name. Skipping.");
		}
	}

	std::shared_ptr<Material> MaterialRegistry::getMaterial(std::string name) {
		auto iter = s_materials.find(name);
		if (iter != s_materials.end()) {
			return iter->second;
		}
		return {};
	}

	void MaterialRegistry::loadMaterials() {
		logger::info("Loading materials...");
		std::string materialPath = "res/materials";
		for (const auto& entry : std::filesystem::directory_iterator(materialPath)) {
			if (entry.is_regular_file()) {
				registerMaterial(entry.path().stem().string(), Material::loadFromJSON(entry.path().string()));
			}
		}

		getMaterial("pbr")->setDefaultTexture(3, ComputeCookTorranceLUT(512));
	}

	void MaterialRegistry::reloadMaterials() {
		for (auto& material : s_materials) {
			material.second->getShaderProgram().reload();
		}
	}

}