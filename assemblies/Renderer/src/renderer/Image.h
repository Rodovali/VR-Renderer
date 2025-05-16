// VR Renderer - Image
// Rodolphe VALICON
// 2025

#pragma once

#include <glad/glad.h>

#include <cstdint>
#include <memory>

namespace vr {

	struct Image {
		uint32_t width;
		uint32_t height;
		GLenum pixelFormat;
		GLenum pixelType;
		std::unique_ptr<uint8_t[]> pixels;
	};

}