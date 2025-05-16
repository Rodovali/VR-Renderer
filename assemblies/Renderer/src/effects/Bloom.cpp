// VR Renderer - Bloom Effect
// Rodolphe VALICON
// 2025

#include "Bloom.h"

namespace vr {
	
	Bloom::Bloom(std::shared_ptr<gpu::Texture> lensDirt) : m_cTexture(GL_TEXTURE_2D), m_dTexture(GL_TEXTURE_2D), m_uTexture(GL_TEXTURE_2D) {
		m_copyShader = std::make_unique<gpu::ShaderProgram>("res/shaders/effects/bloom_copy.glsl");
		m_downsampleShader = std::make_unique<gpu::ShaderProgram>("res/shaders/effects/bloom_downsample.glsl");
		m_upsampleShader = std::make_unique<gpu::ShaderProgram>("res/shaders/effects/bloom_upsample.glsl");
		m_mixShader = std::make_unique<gpu::ShaderProgram>("res/shaders/effects/bloom_mix.glsl");

		m_lensDirt = lensDirt;
	}
	
	void Bloom::apply(RenderTarget& target) {
		// Check if texture needs to be recreated
		int32_t tWidth = target.getWidth();
		int32_t tHeight = target.getHeight();

		if (tWidth != m_width || tHeight != m_height) {
			m_width = tWidth;
			m_height = tWidth;
			
			// Compute max level
			const int32_t reqLevels = 8;
			
			const float minDim = static_cast<float>(m_width < m_height ? m_width : m_height) / 2.0f;
			const int32_t maxLevels = static_cast<int32_t>(std::floorf(std::log2(minDim))) + 1;
			m_levels = maxLevels < reqLevels ? maxLevels : reqLevels;

			// Allocate textures
			m_cTexture = gpu::Texture(GL_TEXTURE_2D);
			glTextureStorage2D(m_cTexture, 1, GL_RGBA16F, m_width, m_height);
			glTextureParameteri(m_cTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_cTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_cTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(m_cTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			m_dTexture = gpu::Texture(GL_TEXTURE_2D);
			glTextureStorage2D(m_dTexture, m_levels, GL_RGBA16F, m_width / 2, m_height / 2);
			glTextureParameteri(m_dTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_dTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_dTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			glTextureParameteri(m_dTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			m_uTexture = gpu::Texture(GL_TEXTURE_2D);
			glTextureStorage2D(m_uTexture, m_levels - 1, GL_RGBA16F, m_width / 2, m_height / 2);
			glTextureParameteri(m_uTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_uTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_uTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			glTextureParameteri(m_uTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		
		int32_t uDMip = glGetUniformLocation(*m_downsampleShader, "uMip");
		int32_t uUMip = glGetUniformLocation(*m_upsampleShader, "uMip");

		// Copy pass
		glUseProgram(*m_copyShader);
		glBindTextureUnit(0, *target.getColorTexture());
		glBindImageTexture(1, m_cTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		glDispatchCompute(m_width / 8, m_height / 8, 1);

		// Downsample passes
		glUseProgram(*m_downsampleShader);
		
		int32_t dFactor = 2;
		glBindTextureUnit(0, m_cTexture);
		glBindImageTexture(0, m_dTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		glUniform1i(uDMip, 0);
		glDispatchCompute(m_width / dFactor / 8, m_height / dFactor / 8, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		for (int32_t mip = 1; mip < m_levels; ++mip) {
			dFactor *= 2;

			glBindTextureUnit(0, m_dTexture);
			glBindImageTexture(0, m_dTexture, mip, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
			glUniform1i(uDMip, mip - 1);
			glDispatchCompute(m_width / dFactor / 8 + 1, m_height / dFactor / 8 + 1, 1);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		}

		// Upsample passes
		dFactor /= 2;

		glUseProgram(*m_upsampleShader);
		glBindTextureUnit(0, m_dTexture);
		glBindImageTexture(0, m_dTexture, m_levels - 2, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
		glBindImageTexture(1, m_uTexture, m_levels - 2, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);

		glUniform1i(uUMip, m_levels - 1);
		glDispatchCompute(m_width / dFactor / 8 + 2, m_height / dFactor / 8 +2, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		for (int32_t mip = m_levels - 3; mip >= 0; --mip) {
			dFactor /= 2;

			glBindTextureUnit(0, m_uTexture);
			glBindImageTexture(0, m_dTexture, mip, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
			glBindImageTexture(1, m_uTexture, mip, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);

			glUniform1i(uUMip, mip + 1);
			glDispatchCompute(m_width / dFactor / 8 + 2, m_height / dFactor / 8 +2, 1);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		}

		// Mix pass
		glUseProgram(*m_mixShader);
		glBindTextureUnit(0, m_uTexture);
		glBindTextureUnit(1, *m_lensDirt);
		glBindImageTexture(0, m_cTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
		glBindImageTexture(1, *target.getColorTexture(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		glDispatchCompute(target.getWidth() / 8 + 1, target.getHeight() / 8 + 1, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	}

	void Bloom::setAmount(float amount) {
		int32_t location = glGetUniformLocation(*m_mixShader, "uBloomAmount");
		glProgramUniform1f(*m_mixShader, location, amount);
	}

}