// VR Renderer - Post processing effect interface
// Rodolphe VALICON
// 2025

#pragma once

#include "renderer/RenderTarget.h"

namespace vr {

	class Effect {
	public:
		virtual void apply(RenderTarget& target) = 0;
	};

}