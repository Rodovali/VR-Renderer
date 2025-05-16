// VR Renderer - Window
// Rodolphe VALICON
// 2025

#pragma once

#include <cstdint>
#include <memory>

// NOTE: Forward declaration of GLFW window's opaque type, to avoid global namespace pollution with the inclusion of this header.
struct GLFWwindow;

namespace vr {
	class WindowSubsystem;

	/// @brief Encapsulates the logic behind window management.
	/// Exposes a simple API to interract with OS.
	class Window final {
		friend class WindowSubsystem;
	
	public:
		///	@brief Returns whenether the windowing system has received a close request
		/// signal for this window instance.
		/// @return true if the window should close, false otherwise.
		bool shouldClose() const;

		/// @brief Polls OS for window events.
		void pollEvents() const;

		/// @brief Swap the window's framebuffers.
		void swapBuffers() const;

		/// @brief Changes the cursor mode of the window.
		/// If mouse capture is enabled, the cursor is invisible and can't leave the window.
		/// @param enable Flag to enable mouse capture.
		void setMouseCapture(bool enable);

		/// @brief Set whenether the window should broadcast key events for key repeats.
		/// @param enable Flag to enable broadcasting key repat
		void allowKeyboardRepeat(bool enable);

		/// @brief Provides current OS time since window subsystem's initialization.
		/// @return Elapsed time since initialization, in seconds.
		float getTime() const;

		/// @brief Provides the window's width.
		/// @return The window's inner width, in pixels.
		int32_t getWidth() { return m_width; }

		/// @brief Provides the window's height.
		/// @return The window's inner height, in pixels.
		int32_t getHeight() { return m_height; }

		/// @brief Provides the internal window instance's GLFW handle.
		/// @returns The GLFW window's GLFW handle.
		GLFWwindow* getGLFWHandle() { return m_windowHandle; }
	
		~Window();
	private:
		Window(int32_t width, int32_t height, const char* title);
		
		static void closeCallback(GLFWwindow* window);
		static void resizeCallback(GLFWwindow* window, int32_t width, int32_t height);
		static void cursorPosCallback(GLFWwindow* window, double x, double y);
		static void mouseButtonCallback(GLFWwindow* window, int32_t button, int32_t action, int32_t mods);
		static void keyCallback(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods);
	private:
		GLFWwindow* m_windowHandle;
		int32_t m_width;
		int32_t m_height;
		bool m_broadcastKeyRepeat = false;
	};

}

