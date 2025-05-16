// VR Renderer - Image loader
// Rodolphe VALICON
// 2025

#include "ImageLoader.h"

#include "core/Logger.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace vr {

	std::shared_ptr<Image> utils::loadImage(const std::string& filePath, GLenum type, bool flip) {
		int32_t width;
		int32_t height;
		int32_t channels;

		stbi_set_flip_vertically_on_load(flip);

		size_t channelSize;
		void* data;
		switch (type) {
		case GL_UNSIGNED_BYTE:
			data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);
			channelSize = 1;
			break;
		case GL_FLOAT:
			data = stbi_loadf(filePath.c_str(), &width, &height, &channels, 0);
			channelSize = 4;
			break;
		default:
			logger::error("Failed to load image '{}': Unsuported data type", filePath);
			return {};
		}

		if (!data) {
			logger::error("Failed to load image '{}': {}", filePath, stbi_failure_reason());
			return {};
		}

		GLenum pixelFormat = 0;
		switch (channels) {
		case 1: pixelFormat = GL_RED; break;
		case 2: pixelFormat = GL_RG; break;
		case 3: pixelFormat = GL_RGB; break;
		case 4: pixelFormat = GL_RGBA; break;
		}

		std::shared_ptr<Image> image = std::make_shared<Image>();
		image->width = width;
		image->height = height;
		image->pixelType = type;
		image->pixelFormat = pixelFormat;

		size_t dataSize = width * height * channels * channelSize;
		image->pixels = std::make_unique<uint8_t[]>(dataSize);
		std::memcpy(image->pixels.get(), data, dataSize);

		stbi_image_free(data);

		logger::info("Loaded image '{}' ({}x{}x{})", filePath, width, height, channels);

		return image;
	}

}