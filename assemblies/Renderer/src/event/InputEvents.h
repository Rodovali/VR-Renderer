// VR Renderer - Input Events
// Rodolphe VALICON
// 2025

#pragma once

#include "event/Event.h"

#include <format>

namespace vr {
	class Window;
	namespace events {

		/// @brief A keyboard key has been triggered.
		class KeyEvent final : public Event {
			friend class Window;
			VR_EVENT_CLASS(KeyEvent)
		public:
			virtual std::string toString() const override {
				return std::format("{} ({}|{}) {}", getName(), key, pressed ? "DOWN" : "UP", repeat ? "REPEAT" : "");
			}

		private:
			KeyEvent(int32_t key, bool pressed, bool repeat) : key(key), pressed(pressed), repeat(repeat) {}

		public:
			const int32_t key;
			const bool pressed;
			const bool repeat;
		};

		/// @brief A mouse button has been triggered.
		class MouseButtonEvent final : public Event {
			friend class Window;
			VR_EVENT_CLASS(MouseButtonEvent)
		public:
			virtual std::string toString() const override {
				return std::format("{} ({}|{})", getName(), button, pressed ? "DOWN" : "UP");
			}

		private:
			MouseButtonEvent(int32_t button, bool pressed) : button(button), pressed(pressed) {}

		public:
			const int32_t button;
			const bool pressed;
		};

		/// @brief The mouse has been moved.
		class MouseMovedEvent final : public Event {
			friend class Window;
			VR_EVENT_CLASS(MouseMovedEvent)
		public:
			virtual std::string toString() const override {
				return std::format("{} ({}, {})", getName(), x, y);
			}

		private:
			MouseMovedEvent(float x, float y) : x(x), y(y) {};

		public:
			const float x;
			const float y;
		};

	}
}

