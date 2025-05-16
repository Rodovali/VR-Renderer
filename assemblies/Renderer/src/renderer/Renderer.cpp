// VR Renderer - Renderer
// Rodolphe VALICON
// 2025

#include "Renderer.h"

#include "gpu/VertexLayout.h"
#include "renderer/MaterialRegistry.h"

#include <glad/glad.h>

namespace vr {
	std::unique_ptr<gpu::VertexArray> Renderer::s_renderVertexArray;
	std::unique_ptr<RenderTarget> Renderer::s_intermediateTarget;
	std::unique_ptr<gpu::ShaderProgram> Renderer::s_shadowMapShader;
	std::unique_ptr<gpu::ShaderProgram> Renderer::s_shadowCubeMapShader;

	Renderer::Renderer(std::weak_ptr<RenderTarget> target) : m_target(target) {
		
		// Prepare scene matrices and matrix buffer
		m_matrixBuffer = gpu::Buffer(sizeof(Matrices), GL_DYNAMIC_DRAW);
		m_directionalLightBuffer = gpu::Buffer(sizeof(DirectionalLight), GL_DYNAMIC_DRAW);
		m_pointLightBuffer = gpu::Buffer(sizeof(PointLight), GL_DYNAMIC_DRAW);
		m_matrices = {};

		// Prepare shadow map and cubemap texture arrays and framebuffer
		gpu::Sampler sampler;
		sampler.magFilter = GL_LINEAR;
		sampler.minFilter = GL_LINEAR;
		sampler.wrapS = GL_CLAMP_TO_BORDER;
		sampler.wrapT = GL_CLAMP_TO_BORDER;
		sampler.wrapR = GL_CLAMP_TO_BORDER;
		
		m_shadowMap = std::make_unique<gpu::Texture>(GL_TEXTURE_2D_ARRAY, sampler);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTextureParameterfv(*m_shadowMap, GL_TEXTURE_BORDER_COLOR, borderColor);
		glTextureStorage3D(*m_shadowMap, 1, GL_DEPTH_COMPONENT32F, m_SHADOW_SIZE, m_SHADOW_SIZE, m_MAX_SHADOW);

		m_shadowCubeMap = std::make_unique<gpu::Texture>(GL_TEXTURE_CUBE_MAP_ARRAY, sampler);
		glTextureParameterfv(*m_shadowCubeMap, GL_TEXTURE_BORDER_COLOR, borderColor);
		glTextureStorage3D(*m_shadowCubeMap, 1, GL_DEPTH_COMPONENT32F, m_SHADOW_SIZE / 4, m_SHADOW_SIZE / 4, m_MAX_SHADOW * 6);


		m_shadowFramebuffer = std::make_unique<gpu::Framebuffer>();
	}

	void Renderer::init(int32_t width, int32_t height) {
		// Prepare rendering quad
		float quadVertices[] = {
			-1.0f, -1.0f, 0.0f, 0.0f,
			 1.0f, -1.0f, 1.0f, 0.0f,
			 1.0f,  1.0f, 1.0f, 1.0f,
			-1.0f,  1.0f, 0.0f, 1.0f,
		};

		gpu::GeometryData quadGeometry{
			.layout{
				{ gpu::Attribute::Position, GL_FLOAT, 2 },
				{ gpu::Attribute::TexCoord0, GL_FLOAT, 2},
			},
			.vertex_data{ reinterpret_cast<uint8_t*>(quadVertices), reinterpret_cast<uint8_t*>(quadVertices + 16) },
			.indices{ 0, 1, 2, 2, 3, 0 },
			.topology = GL_TRIANGLES,
		};

		s_renderVertexArray = std::make_unique<gpu::VertexArray>(quadGeometry);
		s_shadowMapShader = std::make_unique<gpu::ShaderProgram>("res/shaders/renderpasses/shadowMap.glsl");
		s_shadowCubeMapShader = std::make_unique<gpu::ShaderProgram>("res/shaders/renderpasses/shadowCubeMap.glsl");

		adapt(width, height);
	}

	void Renderer::adapt(int32_t width, int32_t height) {
		s_intermediateTarget = std::make_unique<RenderTarget>(width, height);
	}

	void Renderer::beginScene(const Camera& camera) {
		if (auto target = m_target.lock()) {
			m_matrices.eyePosition = camera.eyePos;
			m_matrices.viewTransform = camera.getViewMatrix();
			m_matrices.projectionTransform = camera.getProjectionMatrix();
			glNamedBufferSubData(m_matrixBuffer, 0, sizeof(Matrices), &m_matrices);

			glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_matrixBuffer);

			glBindFramebuffer(GL_FRAMEBUFFER, target->getFramebuffer());
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClearDepth(1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);

			glViewport(0, 0, target->getWidth(), target->getHeight());
		}
	}

	void Renderer::submit(const Scene& scene) {
		// Upload and bind scene light
		int32_t dirLightCap;
		glGetNamedBufferParameteriv(m_directionalLightBuffer, GL_BUFFER_SIZE, &dirLightCap);
		dirLightCap /= sizeof(DirectionalLight);

		int32_t dirLightCount = static_cast<int32_t>(scene.directionalLights.size());
		if (dirLightCap != dirLightCount) {
			// Reallocate buffer to match light count
			m_directionalLightBuffer = gpu::Buffer(dirLightCount * sizeof(DirectionalLight), GL_DYNAMIC_DRAW);
		}

		glNamedBufferSubData(m_directionalLightBuffer, 0, dirLightCount * sizeof(DirectionalLight), scene.directionalLights.data());
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_directionalLightBuffer);

		int32_t ptLightCap;

		int32_t ptLightCount = static_cast<int32_t>(scene.pointLights.size());
		glGetNamedBufferParameteriv(m_pointLightBuffer, GL_BUFFER_SIZE, &ptLightCap);
		ptLightCap /= sizeof(PointLight);
		if (ptLightCap != ptLightCount) {
			// Reallocate buffer to match light count
			m_pointLightBuffer = gpu::Buffer(ptLightCount * sizeof(PointLight), GL_DYNAMIC_DRAW);
		}

		glNamedBufferSubData(m_pointLightBuffer, 0, ptLightCount * sizeof(PointLight), scene.pointLights.data());
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_pointLightBuffer);

		// Shadow Pass
		renderShadowMap(scene);
		glBindTextureUnit(1, *m_shadowMap);
		glBindTextureUnit(2, *m_shadowCubeMap);

		if (auto target = m_target.lock()) {
			glBindFramebuffer(GL_FRAMEBUFFER, target->getFramebuffer());
			glViewport(0, 0, target->getWidth(), target->getHeight());
		}
		
		// Bind skybox environment map
		if (scene.skybox)
			glBindTextureUnit(0, scene.skybox->getCubeMap());

		// Model Pass
		for (auto& mesh : scene.meshes) {
			m_matrices.modelTransform = mesh->transform.getModelMatrix();
			m_matrices.normalTransform = mesh->transform.getNormalMatrix();
			glNamedBufferSubData(m_matrixBuffer, 0, sizeof(Matrices), &m_matrices);

			for (const Primitive& primitive : mesh->primitives) {
				primitive.material->use();
				glBindVertexArray(*primitive.vertexArray);
				glDrawElements(primitive.vertexArray->getTopology(), primitive.vertexArray->getElementCount(), GL_UNSIGNED_INT, nullptr);
			}
		}

		// Skybox Pass
		if (scene.skybox) {
			scene.skybox->material->use();
			glBindVertexArray(*scene.skybox->vertexArray);
			glDrawElements(GL_TRIANGLES, scene.skybox->vertexArray->getElementCount(), GL_UNSIGNED_INT, nullptr);
		}
	}

	void Renderer::endScene() {
		if (auto target = m_target.lock()) {
			// Multi sample to single sample
			glBlitNamedFramebuffer(
				target->getFramebuffer(), s_intermediateTarget->getFramebuffer(),
				0, 0, target->getWidth(), target->getHeight(),
				0, 0, s_intermediateTarget->getWidth(), s_intermediateTarget->getHeight(),
				GL_COLOR_BUFFER_BIT, GL_NEAREST
			);
		}
	}

	void Renderer::postprocess(Effect& effect) {
		effect.apply(*s_intermediateTarget);
	}

	void Renderer::display(const gpu::ShaderProgram& screenShader) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_DEPTH_TEST);
		glUseProgram(screenShader);
		glBindTextureUnit(0, *s_intermediateTarget->getColorTexture());
		glBindVertexArray(*s_renderVertexArray);
		glDrawElements(GL_TRIANGLES, s_renderVertexArray->getElementCount(), GL_UNSIGNED_INT, nullptr);
	}

	void Renderer::renderShadowMap(const Scene& scene) {
		glBindFramebuffer(GL_FRAMEBUFFER, *m_shadowFramebuffer);
		// Directional lights
		glCullFace(GL_FRONT);
		glUseProgram(*s_shadowMapShader);
		glViewport(0, 0, m_SHADOW_SIZE, m_SHADOW_SIZE);

		for (uint32_t i = 0; i < scene.directionalLights.size() && i < m_MAX_SHADOW; ++i) {
			glUniform1ui(glGetUniformLocation(*s_shadowMapShader, "uLightIndex"), i);
			glNamedFramebufferTextureLayer(*m_shadowFramebuffer, GL_DEPTH_ATTACHMENT, *m_shadowMap, 0, i);
			glClear(GL_DEPTH_BUFFER_BIT);

			// Compute models depth
			for (auto& mesh : scene.meshes) {
				m_matrices.modelTransform = mesh->transform.getModelMatrix();
				m_matrices.normalTransform = mesh->transform.getNormalMatrix();
				glNamedBufferSubData(m_matrixBuffer, 0, sizeof(Matrices), &m_matrices);

				for (const Primitive& primitive : mesh->primitives) {
					glBindVertexArray(*primitive.vertexArray);
					glDrawElements(primitive.vertexArray->getTopology(), primitive.vertexArray->getElementCount(), GL_UNSIGNED_INT, nullptr);
				}
			}
		}


		// Point lights
		static glm::mat4 lightProj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
		glViewport(0, 0, m_SHADOW_SIZE / 4, m_SHADOW_SIZE / 4);
		glUseProgram(*s_shadowCubeMapShader);
		int32_t uViewProjLocation = glGetUniformLocation(*s_shadowCubeMapShader, "uLightViewProj");
		
		glCullFace(GL_BACK);
		for (uint32_t i = 0; i < scene.pointLights.size() && i < m_MAX_SHADOW; ++i) {
			const PointLight& light = scene.pointLights[i];
			glUniform1ui(glGetUniformLocation(*s_shadowCubeMapShader, "uLightIndex"), i);

			glm::mat4 viewMatrices[6] = {
				glm::lookAt(light.position, light.position + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
				glm::lookAt(light.position, light.position + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
				glm::lookAt(light.position, light.position + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
				glm::lookAt(light.position, light.position + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f, 0.0f,-1.0f)),
				glm::lookAt(light.position, light.position + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
				glm::lookAt(light.position, light.position + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			};

			
			for (uint32_t face = 0; face < 6; ++face) {
				glm::mat4 viewProj = lightProj * viewMatrices[face];
				glUniformMatrix4fv(uViewProjLocation, 1, GL_FALSE, glm::value_ptr(viewProj));
				glNamedFramebufferTextureLayer(*m_shadowFramebuffer, GL_DEPTH_ATTACHMENT, *m_shadowCubeMap, 0, i * 6 + face);
				glClear(GL_DEPTH_BUFFER_BIT);
				
				// Compute models depth
				for (auto& mesh : scene.meshes) {
					m_matrices.modelTransform = mesh->transform.getModelMatrix();
					m_matrices.normalTransform = mesh->transform.getNormalMatrix();
					glNamedBufferSubData(m_matrixBuffer, 0, sizeof(Matrices), &m_matrices);

					for (const Primitive& primitive : mesh->primitives) {
						glBindVertexArray(*primitive.vertexArray);
						glDrawElements(primitive.vertexArray->getTopology(), primitive.vertexArray->getElementCount(), GL_UNSIGNED_INT, nullptr);
					}
				}
			}
		}
	}

}