// VR Renderer - Render Flags
// Rodolphe VALICON
// 2025

#pragma once

#include <glad/glad.h>

namespace vr {

	struct RenderFlags {
		bool cullingEnable = true;
		GLenum depthFunc = GL_LESS;

		void apply();
	};

}