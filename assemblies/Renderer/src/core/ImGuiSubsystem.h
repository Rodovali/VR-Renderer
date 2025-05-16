// VR Renderer - ImGui Subsystem
// Rodolphe VALICON
// 2025

#pragma once

#include "core/Window.h"
#include "event/Event.h"

namespace vr {
	class Application;

	class ImGuiSubsystem final {
		friend class Application;
	
	public:
		~ImGuiSubsystem();
	
	private:
		ImGuiSubsystem(Window& window);
		

		void beginFrame() const;
		void endFrame() const;

		void onEvent(const Event& event);
	};

}

