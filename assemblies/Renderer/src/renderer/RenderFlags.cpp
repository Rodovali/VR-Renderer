// VR Renderer - Render Flags
// Rodolphe VALICON
// 2025

#include "RenderFlags.h"

namespace vr {

	void RenderFlags::apply() {
		if (cullingEnable) {
			glEnable(GL_CULL_FACE);
		} else {
			glDisable(GL_CULL_FACE);
		}

		glDepthFunc(depthFunc);
	}

}