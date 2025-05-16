// VR Renderer - Application
// Rodolphe VALICON
// 2025

#include "Application.h"
#include "core/Logger.h"
#include "core/Input.h"
#include "event/EventDispatcher.h"
#include "event/WindowEvents.h"
#include "renderer/MaterialRegistry.h"
#include "renderer/Renderer.h"

#include <stdexcept>

namespace vr {
	Application* Application::s_instance = nullptr;

	Application& Application::getInstance() {
		if (s_instance == nullptr) throw std::logic_error("Application singleton instance is not initialized.");
		return *s_instance;
	}

	void Application::stop() {
		m_isRunning = false;
	}

	void Application::broadcastEvent(const Event& event) {
		EventDispatcher dispatcher(event);
		dispatcher.dispatch<events::WindowCloseEvent>([this](const events::WindowCloseEvent& e) {
			stop();
			return false;
		});
		dispatcher.dispatch<events::WindowResizeEvent>([this](const events::WindowResizeEvent& e) {
			Renderer::adapt(e.width, e.height);
			return false;
		});
		m_imguiSubsystem->onEvent(event);
		input::onEvent(event);
		if(!event.isHandled())
			onEvent(event);
	}

	Application::Application(int32_t width, int32_t height, const char* title) 
		: m_isRunning(false)
	{
		if (s_instance != nullptr) throw std::logic_error("Can't instanciate Application more than once.");

		// Initialize subsystems
		logger::info("Initializing application subsystems...");
		m_windowSubsystem = std::unique_ptr<WindowSubsystem>{ new WindowSubsystem() };
		m_window = m_windowSubsystem->createWindow(width, height, title);
		m_renderContext = std::unique_ptr<RenderContext>{ new RenderContext(*m_window) };
		m_imguiSubsystem = std::unique_ptr<ImGuiSubsystem>{ new ImGuiSubsystem(*m_window) };

		input::init(*m_window);

		MaterialRegistry::loadMaterials();
		Renderer::init(width, height);

		logger::info("Application up and running!");
		s_instance = this;
	}

	Application::~Application() {
		logger::info("Stopping application.");
		s_instance = nullptr;
	}

	void Application::run() {
		float frameTimeCurrent = m_window->getTime();
		float frameTimePrevious = frameTimeCurrent;
		float fixedTime = frameTimeCurrent;

		m_isRunning = true;

		do {
			float deltaTime = frameTimeCurrent - frameTimePrevious;

			while (fixedTime < frameTimeCurrent) {
				onFixedUpdate();
				fixedTime += m_fixedDeltaTime;
			}

			input::update();
			m_window->pollEvents();
			
			
			onUpdate(deltaTime);
			onRender();

			m_imguiSubsystem->beginFrame();
			onUI();
			m_imguiSubsystem->endFrame();

			m_window->swapBuffers();

			frameTimePrevious = frameTimeCurrent;
			frameTimeCurrent = m_window->getTime();
		} while (m_isRunning);
	}

}
