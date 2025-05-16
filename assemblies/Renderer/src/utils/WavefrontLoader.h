// VR Renderer - Wavefront Object file loader
// Rodolphe VALICON
// 2025

#pragma once

#include "gpu/GeometryData.h"

#include <memory>
#include <string>

namespace vr {
	namespace utils {

		std::shared_ptr<gpu::GeometryData> loadWavefrontObj(const std::string& filePath);

	}
}