// VR Renderer - Window subsystem
// Rodolphe VALICON
// 2025

#pragma once

#include "core/Window.h"

#include <cstdint>
#include <memory>

namespace vr {
	class Application;

	/// @brief Encapsulates the window manager state logic.
	class WindowSubsystem final {
		friend class Application;
	
	public:
		~WindowSubsystem();
	
	private:
		WindowSubsystem();
		

		std::unique_ptr<Window> createWindow(uint32_t width, uint32_t height, const char* title) const;
	};

}
