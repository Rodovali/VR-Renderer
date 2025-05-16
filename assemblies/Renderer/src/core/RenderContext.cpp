// VR Renderer - Render Context
// Rodolphe VALICON
// 2025

#include "RenderContext.h"

#include "core/Logger.h"

// Prevents GLFW to include its own OpenGL header.
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>


#ifdef _DEBUG
static void openGLMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* user_param) {
	const char* _source;
	const char* _type;

	switch (source) {
	case GL_DEBUG_SOURCE_API: _source = "API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM: _source = "WINDOW SYSTEM"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: _source = "SHADER COMPILER"; return; // Handled by us;
	case GL_DEBUG_SOURCE_THIRD_PARTY: _source = "THIRD PARTY"; break;
	case GL_DEBUG_SOURCE_APPLICATION: _source = "APPLICATION"; break;
	case GL_DEBUG_SOURCE_OTHER: _source = "OTHER"; break;
	default: _source = "UNKNOWN";
	}

	switch (type) {
	case GL_DEBUG_TYPE_ERROR: _type = "ERROR"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: _type = "DEPRECATED BEHAVIOR"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: _type = "UNDEFINED BEHAVIOR"; break;
	case GL_DEBUG_TYPE_PERFORMANCE: _type = "PERFORMANCE"; break;
	case GL_DEBUG_TYPE_PORTABILITY: _type = "PORTABILITY"; break;
	case GL_DEBUG_TYPE_OTHER: _type = "OTHER"; break;
	case GL_DEBUG_TYPE_MARKER: _type = "MARKER"; break;
	default: _type = "UNKNOWN";
	}

	// In the case of high severity messages, debugbreak to be able to traceback causes.
	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH: vr::logger::error("[OpenGL](HIGH | {1} | {2}): {0}", message, _type, _source); __debugbreak(); break;
	case GL_DEBUG_SEVERITY_MEDIUM: vr::logger::warn("[OpenGL](MEDIUM | {1} | {2}): {0}", message, _type, _source); break;
	case GL_DEBUG_SEVERITY_LOW: vr::logger::warn("[OpenGL](LOW | {1} | {2}): {0}", message, _type, _source); break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: return; // SDEBUG("[OpenGL](NOTIFICATION | {1} | {2}): {0}", message, _type, _source); break;
	default: vr::logger::warn("[OpenGL](UNKNOWN | {1} | {2}): {0}", message, _type, _source);
	}
}

static void initializeOpenGLDebug() {
	std::string version((char*)glGetString(GL_VERSION));
	std::string vendor((char*)glGetString(GL_VENDOR));
	std::string renderer((char*)glGetString(GL_RENDERER));

	vr::logger::info("OPENGL VERSION {}", version);
	vr::logger::info("| Vendor: {}", vendor);
	vr::logger::info("| GPU: {}", renderer);

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(&openGLMessageCallback, nullptr);
}
#endif

namespace vr {
	RenderContext::RenderContext(Window& window) {
		glfwMakeContextCurrent(window.getGLFWHandle());
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
			throw std::runtime_error("glad failed to load OpenGL");

#ifdef _DEBUG
		initializeOpenGLDebug();
#endif

		logger::info("Render context initialized.");
	}

	RenderContext::~RenderContext() {
		glfwMakeContextCurrent(nullptr);
	}
}