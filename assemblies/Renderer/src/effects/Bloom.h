// VR Renderer - Bloom Effect
// Rodolphe VALICON
// 2025

#pragma once

#include "gpu/Texture.h"
#include "gpu/ShaderProgram.h"
#include "effects/Effect.h"

#include <memory>

namespace vr {

	class Bloom : public Effect {
	public:
		Bloom(std::shared_ptr<gpu::Texture> lensDirt);

		virtual void apply(RenderTarget& target) override;

		gpu::Texture& getDTexture() { return m_dTexture; }
		gpu::Texture& getUTexture() { return m_uTexture; }
		gpu::Texture& getCTexture() { return m_cTexture; }
		void reload() {
			m_upsampleShader->reload();
			m_downsampleShader->reload();
			m_copyShader->reload();
			m_mixShader->reload();
		}

		void setAmount(float amount);

	private:
		int32_t m_width = 1, m_height = 1;
		int32_t m_levels = 0;

		gpu::Texture m_cTexture;
		gpu::Texture m_dTexture;
		gpu::Texture m_uTexture;

		std::unique_ptr<gpu::ShaderProgram> m_copyShader;
		std::unique_ptr<gpu::ShaderProgram> m_downsampleShader;
		std::unique_ptr<gpu::ShaderProgram> m_upsampleShader;
		std::unique_ptr<gpu::ShaderProgram> m_mixShader;

		std::shared_ptr<gpu::Texture> m_lensDirt;
	};

}