// VR Renderer - Window subsystem
// Rodolphe VALICON
// 2025

#include "WindowSubsystem.h"
#include "core/Logger.h"

// Prevents GLFW to include its own OpenGL header.
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdexcept>

namespace vr {

	WindowSubsystem::WindowSubsystem() {
		if (glfwInit() == GLFW_FALSE) throw std::runtime_error("Failed to initialize GLFW");
		logger::info("Window subsystem initialized.");
	}

	WindowSubsystem::~WindowSubsystem() {
		glfwTerminate();
	}

	std::unique_ptr<Window> WindowSubsystem::createWindow(uint32_t width, uint32_t height, const char* title) const {
		return std::unique_ptr<Window>{new Window(width, height, title)};
	}

}

