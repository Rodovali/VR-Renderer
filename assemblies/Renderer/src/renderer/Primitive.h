// VR Renderer - Primitive
// Rodolphe VALICON
// 2025

#pragma once

#include "gpu/VertexArray.h"
#include "renderer/MaterialInstance.h"

namespace vr {

	struct Primitive {
		std::shared_ptr<gpu::VertexArray> vertexArray;
		std::shared_ptr<MaterialInstance> material;
	};

}