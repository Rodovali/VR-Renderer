// VR Renderer - Window
// Rodolphe VALICON
// 2025

#include "Window.h"
#include "core/Application.h"
#include "event/WindowEvents.h"
#include "event/InputEvents.h"

// Prevents GLFW to include its own OpenGL header.
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <imgui.h>

#include <stdexcept>

namespace vr {

	Window::Window(int32_t width, int32_t height, const char* title)
		: m_width(width), m_height(height)
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
		if (!window)
			throw std::runtime_error("GLFW failed to create a window.");

		glfwSetWindowUserPointer(window, this);
		glfwSetWindowCloseCallback(window, Window::closeCallback);
		glfwSetWindowSizeCallback(window, Window::resizeCallback);
		glfwSetCursorPosCallback(window, Window::cursorPosCallback);
		glfwSetMouseButtonCallback(window, Window::mouseButtonCallback);
		glfwSetKeyCallback(window, Window::keyCallback);

		glfwSwapInterval(1);

		m_windowHandle = window;
	}

	void Window::closeCallback(GLFWwindow* window) {
		Application::getInstance().broadcastEvent(events::WindowCloseEvent());
	}

	void Window::resizeCallback(GLFWwindow* window, int32_t width, int32_t height) {
		Window* win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		win->m_width = width;
		win->m_height = height;

		Application::getInstance().broadcastEvent(events::WindowResizeEvent(width, height));
	}

	void Window::cursorPosCallback(GLFWwindow* window, double x, double y) {
		// if (ImGui::GetIO().WantCaptureMouse) return;

		float xf = static_cast<float>(x);
		float yf = static_cast<float>(y);
		Application::getInstance().broadcastEvent(events::MouseMovedEvent(xf, yf));
	}

	void Window::mouseButtonCallback(GLFWwindow* window, int32_t button, int32_t action, int32_t mods) {
		// if (ImGui::GetIO().WantCaptureMouse) {
			//Input::Clear();
			//return;
		// }

		bool pressed = (action == GLFW_PRESS);
		Application::getInstance().broadcastEvent(events::MouseButtonEvent(button, pressed));
	}

	void Window::keyCallback(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods) {
		Window* win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		
		// if (ImGui::GetIO().WantCaptureKeyboard) return;
		if (action == GLFW_REPEAT && !win->m_broadcastKeyRepeat) return;

		bool pressed = (action == GLFW_PRESS) || (action == GLFW_REPEAT);
		bool repeat = (action == GLFW_REPEAT);
		Application::getInstance().broadcastEvent(events::KeyEvent(key, pressed, repeat));
	}

	Window::~Window() {
		glfwDestroyWindow(m_windowHandle);
	}

	bool Window::shouldClose() const {
		return static_cast<bool>(glfwWindowShouldClose(m_windowHandle));
	}

	void Window::pollEvents() const {
		glfwPollEvents();
	}

	void Window::swapBuffers() const {
		glfwSwapBuffers(m_windowHandle);
	}

	void Window::setMouseCapture(bool enable) {
		int32_t mode = enable ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL;
		glfwSetInputMode(m_windowHandle, GLFW_CURSOR, mode);
	}

	void Window::allowKeyboardRepeat(bool enable) {
		m_broadcastKeyRepeat = enable;
	}

	float Window::getTime() const {
		return static_cast<float>(glfwGetTime());
	}
}