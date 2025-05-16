// VR Renderer - Image loader
// Rodolphe VALICON
// 2025

#pragma once

#include "renderer/Image.h"

#include <memory>
#include <string>

namespace vr {
	namespace utils {
		std::shared_ptr<Image> loadImage(const std::string& filePath, GLenum type, bool flip = false);
	}
}