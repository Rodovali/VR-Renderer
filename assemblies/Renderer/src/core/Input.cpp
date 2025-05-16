// SplatrGL Renderer - Input
// Rodolphe VALICON
// 2024

#include "Input.h"

#include "core/Application.h"
#include "event/EventDispatcher.h"
#include "event/InputEvents.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

struct KeyboardState {
	bool keys[GLFW_KEY_LAST];
};

struct MouseState {
	float x;
	float y;
	bool captured;
	bool buttons[GLFW_MOUSE_BUTTON_LAST];
};

static KeyboardState s_keyboardCurrent;
static KeyboardState s_keyboardPrevious;
static MouseState s_mouseCurrent;
static MouseState s_mousePrevious;

static bool s_isMouseCaptured;

static bool ProcessKeyEvent(const vr::events::KeyEvent& event) {
	s_keyboardCurrent.keys[event.key] = event.pressed;
	return false;
}

static bool ProcessMouseButtonEvent(const vr::events::MouseButtonEvent& event) {
	s_mouseCurrent.buttons[event.button] = event.pressed;
	return false;
}

static bool ProcessMouseMovedEvent(const vr::events::MouseMovedEvent& event) {
	s_mouseCurrent.x = event.x;
	s_mouseCurrent.y = event.y;
	return false;
}

namespace vr {
	namespace input {

		void init(Window& window) {
			if (glfwRawMouseMotionSupported())
				glfwSetInputMode(window.getGLFWHandle(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		}

		void update() {
			s_keyboardPrevious = s_keyboardCurrent;
			s_mousePrevious = s_mouseCurrent;
		}

		void clear() {
			s_keyboardCurrent = {};
		}

		void captureMouse(bool enable) {
			s_mouseCurrent.captured = enable;

			Application::getInstance().getWindow().setMouseCapture(enable);
		}

		bool isMouseCaptured() {
			return s_mouseCurrent.captured && s_mousePrevious.captured;
		};

		bool isKeyDown(int32_t key) {
			return s_keyboardCurrent.keys[key];
		}

		bool wasKeyDown(int32_t key) {
			return s_keyboardPrevious.keys[key];
		}

		bool isButtonDown(MouseButton button) {
			return s_mouseCurrent.buttons[static_cast<size_t>(button)];
		}

		bool wasButtonDown(MouseButton button) {
			return s_mousePrevious.buttons[static_cast<size_t>(button)];
		}

		void getMousePosition(float& x, float& y) {
			x = s_mouseCurrent.x;
			y = s_mouseCurrent.y;
		}

		void getPreviousMousePosition(float& x, float& y) {
			x = s_mousePrevious.x;
			y = s_mousePrevious.y;
		}

		void getMouseDelta(float& x, float& y) {
			x = s_mouseCurrent.x - s_mousePrevious.x;
			y = s_mouseCurrent.y - s_mousePrevious.y;
		}

		void onEvent(const Event& event) {
			EventDispatcher dispatcher(event);
			dispatcher.dispatch<events::KeyEvent>(ProcessKeyEvent);
			dispatcher.dispatch<events::MouseButtonEvent>(ProcessMouseButtonEvent);
			dispatcher.dispatch<events::MouseMovedEvent>(ProcessMouseMovedEvent);
		}

	}
}