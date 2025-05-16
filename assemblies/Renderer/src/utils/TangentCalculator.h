// VR Renderer - Tangent Calculator
// Rodolphe VALICON
// 2025

#pragma once

#include "gpu/GeometryData.h"

#include <memory>


namespace vr {
	namespace utils {
		std::shared_ptr<gpu::GeometryData> computeTangents(std::shared_ptr<gpu::GeometryData> geometry);
	}
}
