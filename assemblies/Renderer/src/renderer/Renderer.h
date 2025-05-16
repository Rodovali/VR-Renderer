// VR Renderer - Renderer
// Rodolphe VALICON
// 2025

#pragma once

#include "renderer/RenderTarget.h"
#include "renderer/Camera.h"
#include "renderer/Scene.h"
#include "gpu/Buffer.h"
#include "gpu/VertexArray.h"
#include "effects/Effect.h"

#include <memory>

namespace vr {

	class Renderer {
		struct Matrices {
			glm::mat4 modelTransform;
			glm::mat4 normalTransform;
			glm::mat4 viewTransform;
			glm::mat4 projectionTransform;
			glm::vec3 eyePosition;
		};

	public:
		Renderer(std::weak_ptr<RenderTarget> target);

		static void init(int32_t width, int32_t height);
		static void adapt(int32_t width, int32_t heigt);

		void beginScene(const Camera& camera);
		void submit(const Scene& scene);
		void endScene();
		void postprocess(Effect& effect);
		void display(const gpu::ShaderProgram& screenShader);

		std::shared_ptr<gpu::Texture> getIntermediateTexture() { return s_intermediateTarget->getColorTexture(); }
		
	private:
		void renderShadowMap(const Scene& scene);

	private:
		std::weak_ptr<RenderTarget> m_target;
		gpu::Buffer m_matrixBuffer;
		gpu::Buffer m_directionalLightBuffer;
		gpu::Buffer m_pointLightBuffer;
		Matrices m_matrices;

		const uint32_t m_SHADOW_SIZE = 4096;
		const uint32_t m_MAX_SHADOW = 4;
		std::unique_ptr<gpu::Texture> m_shadowMap;
		std::unique_ptr<gpu::Texture> m_shadowCubeMap;
		std::unique_ptr<gpu::Framebuffer> m_shadowFramebuffer;
		

		static std::unique_ptr<gpu::VertexArray> s_renderVertexArray;
		static std::unique_ptr<RenderTarget> s_intermediateTarget;
		static std::unique_ptr<gpu::ShaderProgram> s_shadowMapShader;
		static std::unique_ptr<gpu::ShaderProgram> s_shadowCubeMapShader;
	};

}