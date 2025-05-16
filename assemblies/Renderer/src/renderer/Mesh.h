// VR Renderer - Mesh
// Rodolphe VALICON
// 2025

#pragma once

#include "renderer/Primitive.h"
#include "renderer/Transform.h"

#include <vector>

namespace vr {
	
	struct Mesh {
		std::vector<Primitive> primitives;
		Transform transform;
	};
	
}