// VR Renderer - Skybox
// Rodolphe VALICON
// 2025

#include "Skybox.h"

#include "renderer/Image.h"
#include "renderer/MaterialRegistry.h"
#include "utils/ImageLoader.h"
#include "utils/WavefrontLoader.h"
#include "gpu/Framebuffer.h"
#include "gpu/Renderbuffer.h"
#include "gpu/ShaderProgram.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace vr {
	Skybox::Skybox(const std::string& path, float exposureCorrection) {
		// Load Equirectangular map
		std::shared_ptr<Image> equirectangularImage = utils::loadImage(path, GL_FLOAT, true);

		gpu::Texture equirectangular(GL_TEXTURE_2D);
		glTextureStorage2D(equirectangular, 1, GL_RGB32F, equirectangularImage->width, equirectangularImage->height);
		glTextureSubImage2D(equirectangular, 0,
			0, 0,
			equirectangularImage->width, equirectangularImage->height,
			equirectangularImage->pixelFormat, equirectangularImage->pixelType,
			equirectangularImage->pixels.get()
		);
		glTextureParameteri(equirectangular, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(equirectangular, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(equirectangular, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(equirectangular, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		GLsizei mapSize = equirectangularImage->width / 4;

		// Prepare framebuffer and views
		gpu::Framebuffer framebuffer;
		gpu::Renderbuffer depthBuffer;
		glNamedRenderbufferStorage(depthBuffer, GL_DEPTH_COMPONENT24, mapSize, mapSize);

		glNamedFramebufferRenderbuffer(framebuffer, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glDisable(GL_CULL_FACE);

		const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		const glm::mat4 captureViews[6] = {
			glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
			glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
			glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};

		vertexArray = std::make_unique<gpu::VertexArray>(*utils::loadWavefrontObj("res/models/cube.obj"));
		material = std::make_shared<MaterialInstance>(MaterialRegistry::getMaterial("skybox"));
		material->renderFlags.depthFunc = GL_LEQUAL;
		material->renderFlags.cullingEnable = false;
		
		// Render environment map
		std::shared_ptr<gpu::Texture> environment = std::make_shared<gpu::Texture>(GL_TEXTURE_CUBE_MAP);
		glTextureParameteri(*environment, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(*environment, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(*environment, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTextureParameteri(*environment, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(*environment, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureStorage2D(*environment, 8, GL_RGB32F, mapSize, mapSize);

		gpu::ShaderProgram equiToCubemapShader("res/shaders/pre-render/equirectangular_cubemap.glsl");
		glUseProgram(equiToCubemapShader);
		glUniform1f(glGetUniformLocation(equiToCubemapShader, "uExposureCorrection"), exposureCorrection);
		glBindTextureUnit(0, equirectangular);
		glUniformMatrix4fv(glGetUniformLocation(equiToCubemapShader, "uProjection"), 1, GL_FALSE, reinterpret_cast<const GLfloat*>(&captureProjection));
		
		glViewport(0, 0, mapSize, mapSize);
		for (uint32_t face = 0; face < 6; ++face) {
			glNamedFramebufferTextureLayer(framebuffer, GL_COLOR_ATTACHMENT0, *environment, 0, face);
			glUniformMatrix4fv(glGetUniformLocation(equiToCubemapShader, "uView"), 1, GL_FALSE, reinterpret_cast<const GLfloat*>(&captureViews[face]));

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glBindVertexArray(*vertexArray);
			glDrawElements(GL_TRIANGLES, vertexArray->getElementCount(), GL_UNSIGNED_INT, nullptr);
		}
		glGenerateTextureMipmap(*environment);

		// Render Cook-Torrance irradiance map
		m_cubeMap = std::make_unique<gpu::Texture>(GL_TEXTURE_CUBE_MAP);
		glTextureParameteri(*m_cubeMap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(*m_cubeMap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(*m_cubeMap, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTextureParameteri(*m_cubeMap, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(*m_cubeMap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureStorage2D(*m_cubeMap, 8, GL_RGB16F, mapSize, mapSize);
		glGenerateTextureMipmap(*m_cubeMap);

		gpu::ShaderProgram cookTorranceIrradianceShader("res/shaders/pre-render/cook-torrance_irradiance.glsl");
		glUseProgram(cookTorranceIrradianceShader);
		glBindTextureUnit(0, *environment);
		glUniformMatrix4fv(glGetUniformLocation(cookTorranceIrradianceShader, "uProjection"), 1, GL_FALSE, reinterpret_cast<const GLfloat*>(&captureProjection));

		GLsizei sizeFactor = 1;
		for (uint32_t mip = 0; mip < 8; ++mip) {
			uint32_t tSize = mapSize / sizeFactor;

			gpu::Renderbuffer depthBuffer2;
			glNamedRenderbufferStorage(depthBuffer2, GL_DEPTH_COMPONENT24, tSize, tSize);
			glNamedFramebufferRenderbuffer(framebuffer, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer2);
			glUniform1f(glGetUniformLocation(cookTorranceIrradianceShader, "uRoughness"), static_cast<float>(mip) / 7.0f);
			glViewport(0, 0, tSize, tSize);
			sizeFactor *= 2;
			for (uint32_t face = 0; face < 6; ++face) {
				glNamedFramebufferTextureLayer(framebuffer, GL_COLOR_ATTACHMENT0, *m_cubeMap, mip, face);
				glUniformMatrix4fv(glGetUniformLocation(cookTorranceIrradianceShader, "uView"), 1, GL_FALSE, reinterpret_cast<const GLfloat*>(&captureViews[face]));

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glBindVertexArray(*vertexArray);
				glDrawElements(GL_TRIANGLES, vertexArray->getElementCount(), GL_UNSIGNED_INT, nullptr);
			}
		}
	}

}