// VR Renderer - Application
// Rodolphe VALICON
// 2025

#pragma once

#include "core/WindowSubsystem.h"
#include "core/RenderContext.h"
#include "core/Window.h"
#include "core/ImGuiSubsystem.h"
#include "event/Event.h"

#include <cstdint>
#include <memory>

// NOTE: Forward declaration of the program's entry point to be able to declare it as a friend of the Application class.
int main();

namespace vr {

	/// @brief Encapsulates the application logic.
	/// This is a base class that should be inherited from to implement user logic.
	class Application {
		friend int ::main();
	
	public:
		/// @brief Provides the singleton application instance.
		/// @return A reference to the application instance.
		/// @throws std::logic_error is thrown if the singleton instance pointer is null.
		static Application& getInstance();

		/// @brief Notifies that the application should stop.
		void stop();
	
		/// @brief Provides the application instance's window.
		/// @return A reference to the application's window.
		Window& getWindow() { return *m_window; }

		/// @brief Broadcast an event to all the subsystems of the application.
		/// @param event Event to broadcast.
		void broadcastEvent(const Event& event);

	protected:
		/// @brief Constructs the application instance.
		/// This constructor also initializes the Application's singleton instance pointer.
		/// @param width Window's inner width, in pixels.
		/// @param height Window's inner height, in pixels.
		/// @param title Application title, as a null-terminated C-string.
		/// @throws std::logic_error is thrown if the singleton instance is already initialized.
		/// @throws std::runtime_error is thrown if one of the subsystems fails to initialize.
		Application(int32_t width, int32_t height, const char* title);
		virtual ~Application();

		/// @brief Callback for event handling.
		/// @param event Event to handle.
		virtual void onEvent(const Event& event) {}

		/// @brief Callback for application logic.
		/// @param deltaTime Time elapsed between the two last updates, in seconds.
		virtual void onUpdate(float deltaTime) {}

		/// @brief Callback for fixed delta time application logic.
		virtual void onFixedUpdate() {}

		/// @brief Callback for rendering.
		/// This method may be executed on a separate thread in the future.
		virtual void onRender() {}

		/// @brief Callback for ImGui.
		virtual void onUI() {}

	private:
		void run();

	private:
		static Application* s_instance;
		
		std::unique_ptr<WindowSubsystem> m_windowSubsystem;
		std::unique_ptr<Window> m_window;
		std::unique_ptr<RenderContext> m_renderContext;
		std::unique_ptr<ImGuiSubsystem> m_imguiSubsystem;

		const float m_fixedDeltaTime = 0.5f;
		bool m_isRunning;
	};

	/// @brief Instanciate an Application from user Implementation.
	/// This function should be defined in user code.
	/// @return A pointer to the user application instance, casted to a base Application.
	Application* createApplication();

}
