// VR Renderer - Render Context
// Rodolphe VALICON
// 2025

#pragma once

#include "core/Window.h"

namespace vr {
	class Application;

	class RenderContext final {
		friend class Application;

	public:
		~RenderContext();

	private:
		RenderContext(Window& window);
	};

}
