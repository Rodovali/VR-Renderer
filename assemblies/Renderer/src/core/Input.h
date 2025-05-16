// VR Renderer - Input
// Rodolphe VALICON
// 2025

#pragma once

#include "event/Event.h"
#include "core/Window.h"
#include "core/MouseButton.h"

namespace vr {
	namespace input {

		void init(Window& window);

		void update();
		void clear();

		bool isMouseCaptured();
		void captureMouse(bool enable);

		bool isKeyDown(int32_t key);
		bool wasKeyDown(int32_t key);

		bool isButtonDown(MouseButton button);
		bool wasButtonDown(MouseButton button);

		void getMousePosition(float& x, float& y);
		void getPreviousMousePosition(float& x, float& y);

		void getMouseDelta(float& x, float& y);

		void onEvent(const Event& event);

	}
}
