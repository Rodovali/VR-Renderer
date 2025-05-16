// VR Renderer - Window Events
// Rodolphe VALICON
// 2025

#pragma once

#include "event/Event.h"

#include <format>

namespace vr {
	class Window;

	namespace events {

		/// @brief Window close signal has been received.
		class WindowCloseEvent final : public Event {
			friend class Window;
			VR_EVENT_CLASS(WindowCloseEvent)
		
		public:
			virtual std::string toString() const override {
				return getName();
			}

		private:
			WindowCloseEvent() {}
		};

		/// @brief Window has been resized.
		class WindowResizeEvent final : public Event {
			friend class Window;
			VR_EVENT_CLASS(WindowResizeEvent)
		
		public:
			virtual std::string toString() const override {
				return std::format("{} ({}, {})", getName(), width, height);
			}

		private:
			WindowResizeEvent(uint32_t width, uint32_t height) : width(width), height(height) {};

		public:
			const uint32_t width;
			const uint32_t height;
		};

	}
}